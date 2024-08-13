#include "model_assimp.hpp"
#include <filesystem>
#include <fstream>
#include <map>

#include <darmok/model_assimp.hpp>
#include <darmok/model.hpp>
#include <darmok/image.hpp>
#include <darmok/data.hpp>
#include <darmok/vertex.hpp>
#include <darmok/program_core.hpp>
#include <darmok/string.hpp>
#include <darmok/math.hpp>

#include <assimp/vector3.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

namespace darmok
{
    struct AssimpUtils final
    {
        static inline std::string_view getStringView(const aiString& str) noexcept
        {
            return std::string_view(str.data, str.length);
        }

        static inline glm::mat4 convert(const aiMatrix4x4& from) noexcept
        {
            // the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
            return glm::mat4(
                from.a1, from.b1, from.c1, from.d1,
                from.a2, from.b2, from.c2, from.d2,
                from.a3, from.b3, from.c3, from.d3,
                from.a4, from.b4, from.c4, from.d4
            );
        }

        static inline glm::vec3 convert(const aiVector3D& vec) noexcept
        {
            return glm::vec3(vec.x, vec.y, vec.z);
        }

        static inline glm::vec2 convert(const aiVector2D& vec) noexcept
        {
            return glm::vec2(vec.x, vec.y);
        }

        static inline uint8_t convertColorComp(ai_real v) noexcept
        {
            return 255 * v;
        }

        static inline Color convert(const aiColor4D& c) noexcept
        {
            return Color
            {
                convertColorComp(c.r),
                convertColorComp(c.g),
                convertColorComp(c.b),
                convertColorComp(c.a),
            };
        }

        static inline Color3 convert(const aiColor3D& c) noexcept
        {
            return Color3
            {
                convertColorComp(c.r),
                convertColorComp(c.g),
                convertColorComp(c.b)
            };
        }

        static bool fixModelLoadConfig(IDataLoader& dataLoader, AssimpModelLoadConfig& config)
        {
            if (config.vertexLayout.getStride() > 0)
            {
                return false;
            }
            if (!config.programPath.empty())
            {
                ProgramDefinition def;
                def.read(config.programPath);
                config.vertexLayout = def.vertexLayout;
            }
            if (config.vertexLayout.getStride() == 0)
            {
                auto def = Program::getStandardDefinition(config.standardProgram);
                config.vertexLayout = def.vertexLayout;
            }
            return true;
        }
    };

    bool AssimpSceneLoader::supports(std::string_view name) const noexcept
    {
        Assimp::Importer importer;
        return importer.IsExtensionSupported(std::filesystem::path(name).extension().string());
    }

    unsigned int AssimpSceneLoader::getImporterFlags(const Config& config) noexcept
    {
        auto flags = aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType |
            aiProcess_GenSmoothNormals |
            aiProcess_GenBoundingBoxes |
            // apply UnitScaleFactor to everything
            aiProcess_GlobalScale;

        if (config.leftHanded)
        {
            // assimp (and opengl) is right handed (+Z points towards the camera)
// while bgfx (and darmok and directx) is left handed (+Z points away from the camera)
            flags |= aiProcess_ConvertToLeftHanded;
        }
        if (config.populateArmature)
        {
            flags |= aiProcess_PopulateArmatureData;
        }
        return flags;
    }

    std::shared_ptr<aiScene> AssimpSceneLoader::loadFromFile(const std::filesystem::path& path, const Config& config) const
    {
        Assimp::Importer importer;
        auto ptr = importer.ReadFile(path.string(), getImporterFlags(config));
        if (ptr == nullptr)
        {
            auto err = importer.GetErrorString();
            throw std::runtime_error(err);
        }
        return fixScene(importer);
    }

    std::shared_ptr<aiScene> AssimpSceneLoader::loadFromMemory(const DataView& data, const std::string& name, const Config& config) const
    {
        Assimp::Importer importer;
        auto ptr = importer.ReadFileFromMemory(data.ptr(), data.size(), getImporterFlags(config), name.c_str());
        if (ptr == nullptr)
        {
            auto err = importer.GetErrorString();
            throw std::runtime_error(err);
        }
        return fixScene(importer);
    }

    std::shared_ptr<aiScene> AssimpSceneLoader::fixScene(Assimp::Importer& importer) noexcept
    {
        auto scene = importer.GetOrphanedScene();

        // scale camera clip planes, seems to be an assimp bug
        // https://github.com/assimp/assimp/issues/3240
        auto scale = importer.GetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.0f);
        scale *= importer.GetPropertyFloat(AI_CONFIG_APP_SCALE_KEY, 1.0f);
        for (auto i = 0; i < scene->mNumCameras; i++)
        {
            auto cam = scene->mCameras[i];
            cam->mClipPlaneNear *= scale;
            cam->mClipPlaneFar *= scale;
        }
        return std::shared_ptr<aiScene>(scene);
    }

    AssimpModelLoaderImpl::AssimpModelLoaderImpl(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<IImageLoader> imgLoader) noexcept
        : _dataLoader(dataLoader)
        , _allocator(allocator)
        , _imgLoader(imgLoader)
    {
    }

    void AssimpModelLoaderImpl::setConfig(const Config& config) noexcept
    {
        _config = config;
        AssimpUtils::fixModelLoadConfig(_dataLoader, _config);
    }

    bool AssimpModelLoaderImpl::supports(std::string_view name) const noexcept
    {
        return _sceneLoader.supports(name);
    }

    std::shared_ptr<Model> AssimpModelLoaderImpl::operator()(std::string_view name)
    {
        auto scene = _sceneLoader.loadFromMemory(_dataLoader(name), std::string(name));
        auto model = std::make_shared<Model>();
        auto basePath = std::filesystem::path(name).parent_path().string();

        AssimpModelConverter converter(*scene, basePath, _config, _allocator, _imgLoader);
        converter.update(*model);
        return model;
    }

    AssimpModelConverter::AssimpModelConverter(const aiScene& scene, const std::string& basePath, const Config& config,
        bx::AllocatorI& alloc, OptionalRef<IImageLoader> imgLoader) noexcept
        : _scene(scene)
        , _basePath(basePath)
        , _config(config)
        , _allocator(alloc)
        , _imgLoader(imgLoader)
    {
        // TODO: check that we need this for the inverse bind poses
        _inverseRoot = glm::inverse(AssimpUtils::convert(scene.mRootNode->mTransformation));
    }

    std::vector<std::string> AssimpModelConverter::getTexturePaths(const aiScene& scene) noexcept
    {
        std::vector<std::string> paths;
        for (size_t i = 0; i < scene.mNumMaterials; i++)
        {
            auto& assimpMat = *scene.mMaterials[i];
            for (auto& elm : _materialTextures)
            {
                auto& type = elm.first;
                auto size = assimpMat.GetTextureCount(type);
                for (size_t i = 0; i < size; i++)
                {
                    aiString path("");
                    unsigned int uvindex = 0;
                    ai_real blend = 1.F;
                    aiTextureMapping mapping = aiTextureMapping::aiTextureMapping_UV;
                    aiTextureOp operation = aiTextureOp::aiTextureOp_Multiply;
                    assimpMat.GetTexture(type, i, &path,
                        &mapping,
                        &uvindex,
                        &blend,
                        &operation
                    );
                    auto embeddedTex = scene.GetEmbeddedTexture(path.C_Str());
                    if (!embeddedTex)
                    {
                        paths.emplace_back(path.C_Str());
                    }
                }
            }
        }
        return paths;
    }

    AssimpModelConverter& AssimpModelConverter::setBoneNames(const std::vector<std::string>& names) noexcept
    {
        _boneNames.clear();
        for (auto& name : names)
        {
            _boneNames.emplace(name, name);
        }
        return *this;
    }

    AssimpModelConverter& AssimpModelConverter::setBoneNames(const std::unordered_map<std::string, std::string>& names) noexcept
    {
        _boneNames = names;
        return *this;
    }

    AssimpModelConverter& AssimpModelConverter::setConfig(const nlohmann::json& config) noexcept
    {
        if (config.contains("bones"))
        {
            auto& bonesConfig = config["bones"];
            if (bonesConfig.is_array())
            {
                std::vector<std::string> boneNames = bonesConfig;
                setBoneNames(boneNames);
            }
            else
            {
                std::unordered_map<std::string, std::string> boneNames = bonesConfig;
                setBoneNames(boneNames);
            }
        }
        return *this;
    }

    void AssimpModelConverter::update(Model& model) noexcept
    {
        update(model.rootNode, *_scene.mRootNode);
    }

    void AssimpModelConverter::update(ModelNode& modelNode, const aiNode& assimpNode) noexcept
    {
        modelNode.name = AssimpUtils::getStringView(assimpNode.mName);
        modelNode.transform = AssimpUtils::convert(assimpNode.mTransformation);

        for(size_t i = 0; i < assimpNode.mNumMeshes; i++)
        {
            auto& modelRenderable = modelNode.renderables.emplace_back();
            auto assimpMesh = _scene.mMeshes[assimpNode.mMeshes[i]];
            modelRenderable.mesh = getMesh(assimpMesh);
            auto assimpMaterial = _scene.mMaterials[assimpMesh->mMaterialIndex];
            modelRenderable.material = getMaterial(assimpMaterial);
        }

        for (size_t i = 0; i < _scene.mNumCameras; i++)
        {
            auto assimpCam = _scene.mCameras[i];
            if (assimpCam->mName == assimpNode.mName)
            {
                update(modelNode, *assimpCam);
                break;
            }
        }
        for (size_t i = 0; i < _scene.mNumLights; i++)
        {
            auto assimpLight = _scene.mLights[i];
            if (assimpLight->mName == assimpNode.mName)
            {
                update(modelNode, *assimpLight);
                break;
            }
        }

        for (size_t i = 0; i < assimpNode.mNumChildren; i++)
        {
            auto& modelChild = modelNode.children.emplace_back();
            update(modelChild, *assimpNode.mChildren[i]);
        }
    }

    void AssimpModelConverter::update(ModelNode& modelNode, const aiCamera& assimpCam) noexcept
    {
        auto& camNode = modelNode.children.emplace_back();
        camNode.name = AssimpUtils::getStringView(assimpCam.mName);

        aiMatrix4x4 mat;
        assimpCam.GetCameraMatrix(mat);
        camNode.transform = AssimpUtils::convert(mat);

        auto aspect = assimpCam.mAspect;
        auto fovy = 0.f;
        if (aspect != 0.f)
        {
            auto fovx = assimpCam.mHorizontalFOV;
            fovy = 2.f * atan(tan(0.5f * fovx) * aspect);
        }

        auto& cam = camNode.camera.emplace();
        cam.projection = Math::perspective(fovy, aspect, assimpCam.mClipPlaneNear, assimpCam.mClipPlaneFar);
    }

    void AssimpModelConverter::update(ModelNode& modelNode, const aiLight& assimpLight) noexcept
    {
        auto& lightNode = modelNode.children.emplace_back();
        lightNode.name = AssimpUtils::getStringView(assimpLight.mName);
        auto pos = AssimpUtils::convert(assimpLight.mPosition);
        lightNode.transform = glm::translate(glm::mat4(1), pos);

        if (assimpLight.mType == aiLightSource_POINT)
        {
            auto& pointLight = lightNode.pointLight.emplace();
            pointLight.attenuation = glm::vec3(
                assimpLight.mAttenuationConstant,
                assimpLight.mAttenuationLinear,
                assimpLight.mAttenuationQuadratic
            );
            pointLight.diffuseColor = AssimpUtils::convert(assimpLight.mColorDiffuse);
            pointLight.specularColor = AssimpUtils::convert(assimpLight.mColorSpecular);
        }
        else if (assimpLight.mType == aiLightSource_AMBIENT)
        {
            auto& ambLight = lightNode.ambientLight.emplace();
            ambLight.color = AssimpUtils::convert(assimpLight.mColorAmbient);
        }
    }

    void AssimpModelConverter::update(ModelTexture& modelTex, const aiMaterial& assimpMat, aiTextureType type, unsigned int index) noexcept
    {
        aiString path("");
        unsigned int uvindex = 0;
        ai_real blend = 1.F;
        aiTextureMapping mapping = aiTextureMapping::aiTextureMapping_UV;
        aiTextureOp operation = aiTextureOp::aiTextureOp_Multiply;

        assimpMat.GetTexture(type, index, &path,
            &mapping,
            &uvindex,
            &blend,
            &operation
        );

        modelTex.image = getImage(path.C_Str());

        // assimpMat.Get(AI_MATKEY_BASE_COLOR)

        // TODO: add support for other texture params
    }

    const std::unordered_map<aiTextureType, MaterialTextureType> AssimpModelConverter::_materialTextures =
    {        
        { aiTextureType_DIFFUSE, MaterialTextureType::Diffuse },
        { aiTextureType_SPECULAR, MaterialTextureType::Specular },
        { aiTextureType_NORMALS, MaterialTextureType::Normal },

        // TODO: temporary
        { aiTextureType_UNKNOWN, MaterialTextureType::Diffuse },
        { aiTextureType_BASE_COLOR, MaterialTextureType::Diffuse },
    };

    const std::vector<AssimpModelConverter::AssimpMaterialColor> AssimpModelConverter::_materialColors =
    {
        { AI_MATKEY_COLOR_DIFFUSE, MaterialColorType::Diffuse },
        { AI_MATKEY_COLOR_AMBIENT, MaterialColorType::Ambient },
        { AI_MATKEY_COLOR_SPECULAR, MaterialColorType::Specular },
        { AI_MATKEY_COLOR_EMISSIVE, MaterialColorType::Emissive },
        { AI_MATKEY_COLOR_TRANSPARENT, MaterialColorType::Transparent },
        { AI_MATKEY_COLOR_REFLECTIVE, MaterialColorType::Reflective },
    };

    void AssimpModelConverter::update(ModelMaterial& modelMat, const aiMaterial& assimpMat) noexcept
    {
        modelMat.program = _config.program;
        modelMat.standardProgram = _config.standardProgram;

        for (auto& elm : _materialTextures)
        {
            auto size = assimpMat.GetTextureCount(elm.first);
            auto& modelTextures = modelMat.textures[elm.second];
            for(size_t i = 0; i < size; i++)
            {
                auto& modelTexture = modelTextures.emplace_back();
                update(modelTexture, assimpMat, elm.first, 0);
            }
        }

        float shininess;
        if (assimpMat.Get(AI_MATKEY_SHININESS, shininess))
        {
            modelMat.shininess = shininess;
        }

        float opacity = 1.F;
        assimpMat.Get(AI_MATKEY_OPACITY, opacity);
        aiColor3D aiColor;

        for (auto& elm : _materialColors)
        {
            if (!assimpMat.Get(elm.name, elm.type, elm.idx, aiColor))
            {
                continue;
            }
            auto color = Color(AssimpUtils::convert(aiColor), Colors::getMaxValue() * opacity);
            modelMat.colors[elm.darmokType] = color;
        }
        
        int blendMode = aiBlendMode_Default;
        assimpMat.Get(AI_MATKEY_BLEND_FUNC, blendMode);
        switch (blendMode)
        {
            case aiBlendMode_Additive:
            modelMat.blendMode = ModelMaterialBlendMode::Additive;
            break;
            case aiBlendMode_Default:
            modelMat.blendMode = ModelMaterialBlendMode::Default;
            break;
        }
    }

    Data AssimpModelConverter::createVertexData(const aiMesh& assimpMesh, const std::vector<aiBone*>& bones) const noexcept
    {
        auto vertexCount = assimpMesh.mNumVertices;
        VertexDataWriter writer(_config.vertexLayout, vertexCount, _allocator);

        for(size_t i = 0; i < assimpMesh.mNumVertices; i++)
        {
            if(assimpMesh.mVertices)
            {
                writer.write(bgfx::Attrib::Position, i, AssimpUtils::convert(assimpMesh.mVertices[i]));
            }
            if(assimpMesh.mNormals)
            {
                writer.write(bgfx::Attrib::Normal, i, AssimpUtils::convert(assimpMesh.mNormals[i]));
            }
            if(assimpMesh.mTangents)
            {
                writer.write(bgfx::Attrib::Tangent, i, AssimpUtils::convert(assimpMesh.mTangents[i]));
            }
            if(assimpMesh.mBitangents)
            {
                writer.write(bgfx::Attrib::Bitangent, i, AssimpUtils::convert(assimpMesh.mBitangents[i]));
            }
            for(size_t j = 0; j < AI_MAX_NUMBER_OF_COLOR_SETS; j++)
            {
                if(assimpMesh.mColors[j])
                {
                    auto attrib = (bgfx::Attrib::Enum)(bgfx::Attrib::Color0 + j);
                    writer.write(attrib, i, AssimpUtils::convert(assimpMesh.mColors[j][i]));
                }
            }
            for(size_t j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; j++)
            {
                if(assimpMesh.mTextureCoords[j])
                {
                    auto attrib = (bgfx::Attrib::Enum)(bgfx::Attrib::TexCoord0 + j);
                    writer.write(attrib, i, AssimpUtils::convert(assimpMesh.mTextureCoords[j][i]));
                }
            }
        }
        updateBoneData(bones, writer);
        return writer.finish();
    }

    bool AssimpModelConverter::updateBoneData(const std::vector<aiBone*>& bones, VertexDataWriter& writer) const noexcept
    {
        if (bones.empty())
        {
            return false;
        }
        if (!writer.getLayout().has(bgfx::Attrib::Weight) && !writer.getLayout().has(bgfx::Attrib::Indices))
        {
            return false;
        }

        std::map<size_t, std::vector<std::pair<size_t, float>>> data;
        size_t i = 0;
        for (auto bone : bones)
        {
            for (size_t j = 0; j < bone->mNumWeights; j++)
            {
                auto& weight = bone->mWeights[j];
                if (weight.mWeight > 0.F)
                {
                    data[weight.mVertexId].emplace_back(i, weight.mWeight);
                }
            }
            i++;
        }
        for (auto& [i, vert] : data)
        {
            glm::vec4 weights(1, 0, 0, 0);
            glm::vec4 indices(-1);
            size_t j = 0;
            std::sort(vert.begin(), vert.end(), [](auto& a, auto& b) { return a.second > b.second; });
            for (auto& [index, weight] : vert)
            {
                indices[j] = index;
                weights[j] = weight;
                if (++j > 3)
                {
                    break;
                }
            }
            writer.write(bgfx::Attrib::Indices, i, indices);
            writer.write(bgfx::Attrib::Weight, i, weights);
        }
        return true;
    }

    std::vector<VertexIndex> AssimpModelConverter::createIndexData(const aiMesh& assimpMesh) const noexcept
    {
        size_t size = 0;
        for(size_t i = 0; i < assimpMesh.mNumFaces; i++)
        {
            size += assimpMesh.mFaces[i].mNumIndices;
        }
        std::vector<VertexIndex> indices;
        indices.reserve(size);
        for(size_t i = 0; i < assimpMesh.mNumFaces; i++)
        {
            auto& face = assimpMesh.mFaces[i];
            for(size_t j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }
        return indices;
    }

    void AssimpModelConverter::update(ModelMesh& modelMesh, const aiMesh& assimpMesh) noexcept
    {
        modelMesh.boundingBox.min = AssimpUtils::convert(assimpMesh.mAABB.mMin);
        modelMesh.boundingBox.max = AssimpUtils::convert(assimpMesh.mAABB.mMax);

        std::string name(assimpMesh.mName.C_Str());
        auto skip = false;
        for (auto& regex : _config.skipMeshes)
        {
            if (std::regex_match(name, regex))
            {
                skip = true;
                break;
            }
        }
        if (skip)
        {
            return;
        }

        std::vector<aiBone*> bones;
        for (size_t i = 0; i < assimpMesh.mNumBones; i++)
        {
            auto bone = assimpMesh.mBones[i];
            std::string boneName(bone->mName.C_Str());
            if (!_boneNames.empty())
            {
                auto itr = _boneNames.find(boneName);
                if (itr == _boneNames.end())
                {
                    continue;
                }
                boneName = itr->second;
            }
            bones.push_back(bone);
            modelMesh.joints.push_back(ModelArmatureJoint{
                boneName,
                AssimpUtils::convert(bone->mOffsetMatrix) * _inverseRoot
            });
        }

        modelMesh.vertexData = createVertexData(assimpMesh, bones);
        modelMesh.indexData = createIndexData(assimpMesh);
        modelMesh.vertexLayout = _config.vertexLayout;
    }
    
    std::shared_ptr<ModelMesh> AssimpModelConverter::getMesh(const aiMesh* assimpMesh) noexcept
    {
        auto itr = _meshes.find(assimpMesh);
        if (itr != _meshes.end())
        {
            return itr->second;
        }
        auto modelMesh = std::make_shared<ModelMesh>();
        update(*modelMesh, *assimpMesh);
        _meshes.emplace(assimpMesh, modelMesh);
        return modelMesh;
    }

    std::shared_ptr<ModelMaterial> AssimpModelConverter::getMaterial(const aiMaterial* assimpMat) noexcept
    {
        auto itr = _materials.find(assimpMat);
        if (itr != _materials.end())
        {
            return itr->second;
        }
        auto modelMat = std::make_shared<ModelMaterial>();
        update(*modelMat, *assimpMat);
        _materials.emplace(assimpMat, modelMat);
        return modelMat;
    }

    std::shared_ptr<ModelImage> AssimpModelConverter::getImage(const std::string& path) noexcept
    {
        auto itr = _images.find(path);
        if (itr != _images.end())
        {
            return itr->second;
        }
        std::shared_ptr<Image> img;
        auto assimpTex = _scene.GetEmbeddedTexture(path.c_str());
        if (assimpTex)
        {
            size_t size;
            auto format = bimg::TextureFormat::Count;
            if (assimpTex->mHeight == 0)
            {
                // compressed;
                size = assimpTex->mWidth;
            }
            else
            {
                size = (size_t)assimpTex->mWidth * assimpTex->mHeight * 4;
                format = bimg::TextureFormat::RGBA8;
            }
            auto data = DataView(assimpTex->pcData, size);
            img = std::make_shared<Image>(data, _allocator, format);
        }
        if (!img && _imgLoader)
        {
            std::filesystem::path fsPath(path);
            if (fsPath.is_relative())
            {
                fsPath = _basePath / fsPath;
            }
            img = (*_imgLoader)(fsPath.string());
        }

        auto modelImg = std::make_shared<ModelImage>(
            img->getData(), path, img->getTextureConfig());
        _images.emplace(path, modelImg);
        return modelImg;
    }

    AssimpModelLoader::AssimpModelLoader(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<IImageLoader> imgLoader) noexcept
        : _impl(std::make_unique<AssimpModelLoaderImpl>(dataLoader, allocator, imgLoader))
    {
    }

    AssimpModelLoader::~AssimpModelLoader() noexcept
    {
        // empty to forward declare the impl pointer
    }

    AssimpModelLoader& AssimpModelLoader::setConfig(const Config& config) noexcept
    {
        _impl->setConfig(config);
        return *this;
    }

    AssimpModelLoader::result_type AssimpModelLoader::operator()(std::string_view name)
    {
        return (*_impl)(name);
    }

    AssimpModelImporterImpl::AssimpModelImporterImpl()
        : _dataLoader(_fileReader, _allocator)
        , _imgLoader(_dataLoader, _allocator)
    {
    }

    const std::string AssimpModelImporterImpl::_outputFormatJsonKey = "outputFormat";
    const std::string AssimpModelImporterImpl::_outputPathJsonKey = "outputPath";
    const std::string AssimpModelImporterImpl::_vertexLayoutJsonKey = "vertexLayout";
    const std::string AssimpModelImporterImpl::_embedTexturesJsonKey = "embedTextures";
    const std::string AssimpModelImporterImpl::_programPathJsonKey = "programPath";
    const std::string AssimpModelImporterImpl::_programJsonKey = "program";
    const std::string AssimpModelImporterImpl::_skipMeshesJsonKey = "skipMeshes";

    void AssimpModelImporterImpl::loadConfig(const nlohmann::ordered_json& json, const std::filesystem::path& basePath, LoadConfig& config) const
    {
        if (json.contains(_vertexLayoutJsonKey))
        {
            config.vertexLayout = loadVertexLayout(json[_vertexLayoutJsonKey]);
        }
        if (json.contains(_programPathJsonKey))
        {
            config.programPath = basePath / json[_programPathJsonKey];
        }
        if (json.contains(_programJsonKey))
        {
            std::string val = json[_programJsonKey];
            auto standard = Program::getStandardType(val);
            if (standard)
            {
                config.standardProgram = standard.value();
            }
            else
            {
                config.program = val;
            }
        }
        else if (!config.programPath.empty())
        {
            config.program = StringUtils::getFileStem(config.programPath.string()) + ".bin";
        }

        if (json.contains(_skipMeshesJsonKey))
        {
            auto& jsonVal = json[_skipMeshesJsonKey];
            if (jsonVal.is_array())
            {
                for (auto& elm : jsonVal)
                {
                    config.skipMeshes.emplace_back(StringUtils::globToRegex(elm));
                }
            }
            else
            {
                config.skipMeshes.emplace_back(StringUtils::globToRegex(jsonVal));
            }
        }
        if (json.contains(_embedTexturesJsonKey))
        {
            config.embedTextures = json[_embedTexturesJsonKey];
        }
    }

    void AssimpModelImporterImpl::loadConfig(const nlohmann::ordered_json& json, const std::filesystem::path& basePath, Config& config) const
    {
        loadConfig(json, basePath, config.loadConfig);
        if (json.contains(_outputPathJsonKey))
        {
            config.outputPath = json[_outputPathJsonKey].get<std::string>();
        }
        if (json.contains(_outputFormatJsonKey))
        {
            config.outputFormat = Model::getFormat(json[_outputFormatJsonKey]);
        }
        else if (!config.outputPath.empty())
        {
            config.outputFormat = Model::getExtensionFormat(config.outputPath.extension().string());
        }
    }

    VertexLayout AssimpModelImporterImpl::loadVertexLayout(const nlohmann::ordered_json& json)
    {
        VertexLayout layout;
        if (json.is_string())
        {
            layout.read(json.get<std::filesystem::path>());
        }
        else
        {
            layout.read(json);
        }
        return layout;
    }

    bool AssimpModelImporterImpl::startImport(const Input& input, bool dry)
    {
        if (input.config.is_null())
        {
            return false;
        }
        if (!_assimpLoader.supports(input.path.string()))
        {
            return false;
        }
        auto& config = _currentConfig.emplace();
        nlohmann::json configJson = input.config;
        if (!input.dirConfig.empty())
        {
            configJson.update(input.dirConfig);
        }
        loadConfig(configJson, input.basePath, config);
        AssimpUtils::fixModelLoadConfig(_dataLoader, _currentConfig->loadConfig);
        _currentScene = _assimpLoader.loadFromFile(input.path);
        return _currentScene != nullptr;
    }

    void AssimpModelImporterImpl::endImport(const Input& input)
    {
        _currentConfig.reset();
        _currentScene.reset();
    }

    std::vector<std::filesystem::path> AssimpModelImporterImpl::getOutputs(const Input& input) 
    {
        std::vector<std::filesystem::path> outputs;
        auto outputPath = _currentConfig->outputPath;
        if (outputPath.empty())
        {
            std::string stem = StringUtils::getFileStem(input.path.filename().string());
            outputPath = stem + Model::getFormatExtension(_currentConfig->outputFormat);
        }
        auto basePath = input.getRelativePath().parent_path();
        outputs.push_back(basePath / outputPath);
        return outputs;
    }

    AssimpModelImporterImpl::Dependencies AssimpModelImporterImpl::getDependencies(const Input& input)
    {
        Dependencies deps;
        if (!_currentConfig->loadConfig.embedTextures)
        {
            return deps;
        }
        auto basePath = input.getRelativePath().parent_path();
        for (auto& texPath : AssimpModelConverter::getTexturePaths(*_currentScene))
        {
            deps.insert(basePath / texPath);
        }
        return deps;
    }

    std::ofstream AssimpModelImporterImpl::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path) const
    {
        Config config;
        loadConfig(input.config, input.basePath, config);
        switch (config.outputFormat)
        {
        case OutputFormat::Binary:
            return std::ofstream(path, std::ios::binary);
        default:
            return std::ofstream(path);
        }
    }

    void AssimpModelImporterImpl::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        Model model;
        auto basePath = input.path.parent_path().string();
        OptionalRef<IImageLoader> imgLoader = _imgLoader;
        if (_currentConfig && !_currentConfig->loadConfig.embedTextures)
        {
            imgLoader = nullptr;
        }
        AssimpModelConverter converter(*_currentScene, basePath, _currentConfig->loadConfig, _allocator, imgLoader);
        converter.setConfig(input.config);
        converter.update(model);
        model.write(out, _currentConfig->outputFormat);
    }

    const std::string& AssimpModelImporterImpl::getName() const noexcept
    {
        static const std::string name("model");
        return name;
    }

    AssimpModelImporter::AssimpModelImporter()
        : _impl(std::make_unique<AssimpModelImporterImpl>())
    {
    }

    AssimpModelImporter::~AssimpModelImporter()
    {
        // empty on purpose
    }

    bool AssimpModelImporter::startImport(const Input& input, bool dry)
    {
        return _impl->startImport(input, dry);
    }

    std::vector<std::filesystem::path> AssimpModelImporter::getOutputs(const Input& input)
    {
        return _impl->getOutputs(input);
    }

    AssimpModelImporter::Dependencies AssimpModelImporter::getDependencies(const Input& input)
    {
        return _impl->getDependencies(input);
    }

    std::ofstream AssimpModelImporter::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
    {
        return _impl->createOutputStream(input, outputIndex, path);
    }

    void AssimpModelImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        return _impl->writeOutput(input, outputIndex, out);
    }

    void AssimpModelImporter::endImport(const Input& input)
    {
        return _impl->endImport(input);
    }

    const std::string& AssimpModelImporter::getName() const noexcept
    {
        return _impl->getName();
    }
}