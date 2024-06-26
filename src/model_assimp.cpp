#include "model_assimp.hpp"
#include <filesystem>
#include <fstream>

#include <darmok/model_assimp.hpp>
#include <darmok/model.hpp>
#include <darmok/image.hpp>
#include <darmok/data.hpp>
#include <darmok/vertex.hpp>
#include <darmok/program.hpp>
#include <darmok/program_standard.hpp>
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
        _config.vertexLayout = StandardProgramLoader::getVertexLayout(_config.standardProgram);
    }

    void AssimpModelLoaderImpl::setConfig(const Config& config) noexcept
    {
        _config = config;
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

        AssimpModelConverter ctxt(*scene, basePath, _config, _allocator, _imgLoader);
        ctxt.update(*model);
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
        // TODO: add support for other texture params
    }

    const std::unordered_map<aiTextureType, MaterialTextureType> AssimpModelConverter::_materialTextures =
    {
        { aiTextureType_DIFFUSE, MaterialTextureType::Diffuse },
        { aiTextureType_SPECULAR, MaterialTextureType::Specular },
        { aiTextureType_NORMALS, MaterialTextureType::Normal },
    };

    void AssimpModelConverter::update(ModelMaterial& modelMat, const aiMaterial& assimpMat) noexcept
    {
        modelMat.standardProgram = _config.standardProgram;
        modelMat.programName = _config.programName;
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
    }

    Data AssimpModelConverter::createVertexData(const aiMesh& assimpMesh) const noexcept
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

        if(assimpMesh.HasBones())
        {
            int boneIndex = 0;
            std::vector<int> boneCount(vertexCount);
            std::vector<glm::vec4> boneIndices(vertexCount);
            std::fill(boneIndices.begin(), boneIndices.end(), glm::vec4(-1));
            std::vector<glm::vec4> boneWeights(vertexCount);
            std::fill(boneWeights.begin(), boneWeights.end(), glm::vec4(1, 0, 0, 0));
            for(size_t i = 0; i < assimpMesh.mNumBones; i++)
            {
                auto bone = assimpMesh.mBones[i];
                for(size_t j = 0; j < bone->mNumWeights; j++)
                {
                    auto& weight = bone->mWeights[j];
                    auto i = weight.mVertexId;
                    auto c = boneCount[i]++;
                    if (c <= 3)
                    {
                        boneIndices[i][c] = boneIndex;
                        boneWeights[i][c] = weight.mWeight;
                    }
                }
                ++boneIndex;
            }
            writer.write(bgfx::Attrib::Indices, boneIndices);
            writer.write(bgfx::Attrib::Weight, boneWeights);
        }

        return writer.finish();
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
        modelMesh.vertexData = createVertexData(assimpMesh);
        modelMesh.indexData = createIndexData(assimpMesh);
        modelMesh.vertexLayout = _config.vertexLayout;

        for(size_t i = 0; i < assimpMesh.mNumBones; i++)
        {
            auto bone = assimpMesh.mBones[i];
            modelMesh.joints.push_back(ModelArmatureJoint{
                std::string(bone->mName.C_Str()),
                AssimpUtils::convert(bone->mOffsetMatrix) * _inverseRoot
            });
        }
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
        , _programVertexLayoutSuffix(".vlayout")
    {
    }

    std::filesystem::path AssimpModelImporterImpl::getOutputPath(const std::filesystem::path& path, OutputFormat format) noexcept
    {
        auto stem = std::string(StringUtils::getFileStem(path.filename().string()));
        return path.parent_path() / (stem + Model::getFormatExtension(format));
    }

    const std::string AssimpModelImporterImpl::_outputFormatJsonKey = "outputFormat";
    const std::string AssimpModelImporterImpl::_outputPathJsonKey = "outputPath";
    const std::string AssimpModelImporterImpl::_vertexLayoutJsonKey = "vertexLayout";
    const std::string AssimpModelImporterImpl::_embedTexturesJsonKey = "embedTextures";
    const std::string AssimpModelImporterImpl::_programJsonKey = "program";

    void AssimpModelImporterImpl::loadConfig(const nlohmann::ordered_json& json, const std::filesystem::path& basePath, LoadConfig& config) const
    {
        if (json.contains(_vertexLayoutJsonKey))
        {
            config.vertexLayout = loadVertexLayout(json[_vertexLayoutJsonKey]);
        }
        if (json.contains(_embedTexturesJsonKey))
        {
            config.embedTextures = json[_embedTexturesJsonKey];
        }
        if (json.contains(_programJsonKey))
        {
            std::string progStr = json[_programJsonKey];
            auto standard = StandardProgramLoader::getType(progStr);
            if (standard)
            {
                config.standardProgram = standard.value();
            }
            else
            {
                config.programName = progStr;
            }
        }
        if (config.vertexLayout.getStride() == 0)
        {
            if (config.programName.empty())
            {
                config.vertexLayout = StandardProgramLoader::getVertexLayout(config.standardProgram);
            }
            else
            {
                auto path = basePath / (config.programName + _programVertexLayoutSuffix);
                VertexLayoutUtils::readFile(path, config.vertexLayout);
            }
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

    bgfx::VertexLayout AssimpModelImporterImpl::loadVertexLayout(const nlohmann::ordered_json& json)
    {
        bgfx::VertexLayout layout;
        if (json.is_string())
        {
            std::string str = json;
            auto standard = StandardProgramLoader::getType(str);
            if (standard)
            {
                return StandardProgramLoader::getVertexLayout(standard.value());
            }
            VertexLayoutUtils::readFile(str, layout);
            return layout;
        }
        VertexLayoutUtils::readJson(json, layout);
        return layout;
    }

    bool AssimpModelImporterImpl::startImport(const Input& input, bool dry)
    {
        std::vector<std::filesystem::path> outputs;
        if (input.config.is_null())
        {
            return false;
        }
        if (!_assimpLoader.supports(input.path.string()))
        {
            return false;
        }
        auto& config = _currentConfig.emplace();
        loadConfig(input.config, input.basePath, config);
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
        if (_currentConfig->outputPath.empty())
        {
            outputs.push_back(getOutputPath(input.getRelativePath(), _currentConfig->outputFormat));
        }
        else
        {
            outputs.push_back(_currentConfig->outputPath);
        }
        return outputs;
    }

    std::vector<std::filesystem::path> AssimpModelImporterImpl::getDependencies(const Input& input)
    {
        std::vector<std::filesystem::path> deps;
        if (!_currentConfig->loadConfig.embedTextures)
        {
            return deps;
        }
        auto basePath = input.getRelativePath().parent_path();
        for (auto& texPath : AssimpModelConverter::getTexturePaths(*_currentScene))
        {
            deps.push_back(basePath / texPath);
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
        AssimpModelConverter ctxt(*_currentScene, basePath, _currentConfig->loadConfig, _allocator, _imgLoader);
        ctxt.update(model);
        model.write(out, _currentConfig->outputFormat);
    }

    const std::string& AssimpModelImporterImpl::getName() const noexcept
    {
        static const std::string name("model");
        return name;
    }

    void AssimpModelImporterImpl::setProgramVertexLayoutSuffix(const std::string& suffix)
    {
        _programVertexLayoutSuffix = suffix;
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

    std::vector<std::filesystem::path> AssimpModelImporter::getDependencies(const Input& input)
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