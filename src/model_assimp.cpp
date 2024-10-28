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
#include <darmok/material.hpp>

#include <assimp/vector3.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/GltfMaterial.h>
#include <mikktspace.h>

#include <glm/gtx/component_wise.hpp>

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

        static inline float getIntensity(const aiColor3D& c) noexcept
        {
            return glm::compMax(glm::vec3(c.r, c.g, c.b));
        }

        static inline Color3 convert(aiColor3D c) noexcept
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
                if (!def.empty())
                {
                    config.vertexLayout = def.vertexLayout;
                }
                else
                {
                    VaryingDefinition def;
                    def.read(config.programPath);
                    config.vertexLayout = def.vertex;
                }
                if (config.vertexLayout.getStride() == 0)
                {
                    throw std::runtime_error("failed to load vertex layout from program path");
                }
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
        auto flags = // aiProcess_CalcTangentSpace | // produces weird tangents, we use mikktspace instead
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType |
            aiProcess_GenSmoothNormals |
            aiProcess_GenBoundingBoxes |
            aiProcess_LimitBoneWeights |
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

        // https://github.com/assimp/assimp/issues/3240
        auto scale = importer.GetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.0f);
        scale *= importer.GetPropertyFloat(AI_CONFIG_APP_SCALE_KEY, 1.0f);

        // scale camera clip planes, seems to be an assimp bug
        for (auto i = 0; i < scene->mNumCameras; ++i)
        {
            auto cam = scene->mCameras[i];
            cam->mClipPlaneNear *= scale;
            cam->mClipPlaneFar *= scale;
        }

        // scale light parameters
        for (auto i = 0; i < scene->mNumLights; ++i)
        {
            auto light = scene->mLights[i];
            light->mColorAmbient = light->mColorAmbient * scale;
            light->mColorDiffuse = light->mColorDiffuse * scale;
            light->mColorSpecular = light->mColorSpecular * scale;
            light->mAttenuationLinear /= scale;
            light->mAttenuationQuadratic /= scale * scale;
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
    }

    std::vector<std::string> AssimpModelConverter::getTexturePaths(const aiScene& scene) noexcept
    {
        std::vector<std::string> paths;
        for (size_t i = 0; i < scene.mNumMaterials; ++i)
        {
            auto& assimpMat = *scene.mMaterials[i];
            for (auto& elm : _materialTextures)
            {
                aiString path("");
                if (assimpMat.GetTexture(elm.assimpType, elm.assimpIndex, &path) != AI_SUCCESS)
                {
                    continue;
                }
                auto embeddedTex = scene.GetEmbeddedTexture(path.C_Str());
                if (!embeddedTex)
                {
                    paths.emplace_back(path.C_Str());
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

    bool AssimpModelConverter::updateMeshes(ModelNode& modelNode, const std::regex& regex) noexcept
    {
        auto found = false;
        for (int i = 0; i < _scene.mNumMeshes; ++i)
        {
            auto assimpMesh = _scene.mMeshes[i];
            auto name = assimpMesh->mName.C_Str();
            if (!std::regex_match(name, regex))
            {
                continue;
            }
            modelNode.name = name;
            auto& modelRenderable = modelNode.renderables.emplace_back();
            modelRenderable.mesh = getMesh(assimpMesh);
            auto assimpMaterial = _scene.mMaterials[assimpMesh->mMaterialIndex];
            modelRenderable.material = getMaterial(assimpMaterial);
            found = true;
        }
        return found;
    }

    bool AssimpModelConverter::update(Model& model) noexcept
    {
        if (_config.rootMesh)
        {
            return updateMeshes(model.rootNode, _config.rootMesh.value());
        }
        return update(model.rootNode, *_scene.mRootNode);
    }

    bool AssimpModelConverter::update(ModelNode& modelNode, const aiNode& assimpNode) noexcept
    {
        modelNode.name = AssimpUtils::getStringView(assimpNode.mName);
        for (auto& regex : _config.skipNodes)
        {
            if (std::regex_match(modelNode.name, regex))
            {
                return false;
            }
        }
        // std::cout << modelNode.name << std::endl;

        modelNode.transform = AssimpUtils::convert(assimpNode.mTransformation);

        for(size_t i = 0; i < assimpNode.mNumMeshes; ++i)
        {
            auto& modelRenderable = modelNode.renderables.emplace_back();
            auto assimpMesh = _scene.mMeshes[assimpNode.mMeshes[i]];
            modelRenderable.mesh = getMesh(assimpMesh);
            auto assimpMaterial = _scene.mMaterials[assimpMesh->mMaterialIndex];
            modelRenderable.material = getMaterial(assimpMaterial);
        }

        for (size_t i = 0; i < _scene.mNumCameras; ++i)
        {
            auto assimpCam = _scene.mCameras[i];
            if (assimpCam->mName == assimpNode.mName)
            {
                update(modelNode, *assimpCam);
                break;
            }
        }
        for (size_t i = 0; i < _scene.mNumLights; ++i)
        {
            auto assimpLight = _scene.mLights[i];
            if (assimpLight->mName == assimpNode.mName)
            {
                update(modelNode, *assimpLight);
                break;
            }
        }

        for (size_t i = 0; i < assimpNode.mNumChildren; ++i)
        {
            ModelNode modelChild;
            if (!update(modelChild, *assimpNode.mChildren[i]))
            {
                continue;
            }
            modelNode.children.push_back(std::move(modelChild));
        }

        return true;
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

    float AssimpModelConverter::getLightRadius(const glm::vec3& attenuation) noexcept
    {
        static const float intensityThreshold = 0.001F;
        auto thres = intensityThreshold;

        auto quat = attenuation[2];
        auto lin = attenuation[1];
        auto cons = attenuation[0];

        if (quat == 0.F)
        {
            if (lin == 0)
            {
                return 0.0f;
            }
            return (thres - cons) / lin;
        }

        float disc = lin * lin - 4 * quat * (cons - thres);
        if (disc < 0.0f)
        {
            return 0.0f;
        }

        float d1 = (-lin + sqrt(disc)) / (2 * quat);
        float d2 = (-lin - sqrt(disc)) / (2 * quat);
        return glm::max(d1, d2);
    }

    void AssimpModelConverter::update(ModelNode& modelNode, const aiLight& assimpLight) noexcept
    {
        auto& lightNode = modelNode.children.emplace_back();
        lightNode.name = AssimpUtils::getStringView(assimpLight.mName);
        auto pos = AssimpUtils::convert(assimpLight.mPosition);
        lightNode.transform = glm::translate(glm::mat4(1), pos);

        auto intensity = AssimpUtils::getIntensity(assimpLight.mColorDiffuse);
        auto color = AssimpUtils::convert(assimpLight.mColorDiffuse * (1.F / intensity));

        if (assimpLight.mType == aiLightSource_POINT)
        {
            auto& light = lightNode.pointLight.emplace();
            auto attn = glm::vec3(
                assimpLight.mAttenuationConstant,
                assimpLight.mAttenuationLinear,
                assimpLight.mAttenuationQuadratic
            );
            light.radius = getLightRadius(attn);
            light.intensity = intensity;
            light.color = color;
            // we're not supporting different specular color in lights
        }
        else if (assimpLight.mType == aiLightSource_DIRECTIONAL)
        {
            auto& light = lightNode.dirLight.emplace();
            light.intensity = intensity;
            light.color = color;
            light.direction = AssimpUtils::convert(assimpLight.mDirection);
        }
        else if (assimpLight.mType == aiLightSource_SPOT)
        {
            auto& light = lightNode.spotLight.emplace();
            light.intensity = intensity;
            light.color = color;
            light.innerAngle = assimpLight.mAngleInnerCone;
            light.outerAngle = assimpLight.mAngleOuterCone;
        }
        else if (assimpLight.mType == aiLightSource_AMBIENT)
        {
            auto& ambLight = lightNode.ambientLight.emplace();
            ambLight.intensity = AssimpUtils::getIntensity(assimpLight.mColorAmbient);
            ambLight.color = AssimpUtils::convert(assimpLight.mColorAmbient * (1.F / ambLight.intensity));
        }
    }

    void AssimpModelConverter::update(ModelTexture& modelTex, const aiMaterial& assimpMat, aiTextureType type, unsigned int index) noexcept
    {
        aiString path("");
        if (assimpMat.GetTexture(type, index, &path) == AI_SUCCESS)
        {
            modelTex.image = getImage(path.C_Str());
        }
    }

    const std::vector<AssimpModelConverter::AssimpMaterialTexture> AssimpModelConverter::_materialTextures =
    {        
        { AI_MATKEY_BASE_COLOR_TEXTURE, MaterialTextureType::BaseColor },
        { aiTextureType_DIFFUSE, 0, MaterialTextureType::BaseColor },
        { aiTextureType_SPECULAR, 0, MaterialTextureType::Specular },
        { AI_MATKEY_METALLIC_TEXTURE, MaterialTextureType::MetallicRoughness },
        { AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, MaterialTextureType::MetallicRoughness },
        { aiTextureType_NORMALS, 0, MaterialTextureType::Normal },
        { aiTextureType_AMBIENT_OCCLUSION, 0, MaterialTextureType::Occlusion },
        { aiTextureType_LIGHTMAP, 0, MaterialTextureType::Occlusion },
        { aiTextureType_EMISSIVE, 0, MaterialTextureType::Emissive },
    };

    void AssimpModelConverter::update(ModelMaterial& modelMat, const aiMaterial& assimpMat) noexcept
    {
        modelMat.program = _config.program;
        modelMat.standardProgram = _config.standardProgram;
        modelMat.programDefines.insert(_config.programDefines.begin(), _config.programDefines.end());

        for (auto& elm : _materialTextures)
        {
            auto& modelTexture = modelMat.textures[elm.darmokType];
            update(modelTexture, assimpMat, elm.assimpType, elm.assimpIndex);
        }

        auto& baseColorTexture = modelMat.textures[MaterialTextureType::BaseColor];
        if (baseColorTexture.image == nullptr && !_config.defaultTexture.empty())
        {
            baseColorTexture.image = getImage(_config.defaultTexture);
        }

        // TODO: convert aiTextureType_METALNESS + aiTextureType_DIFFUSE_ROUGHNESS
        // TODO: also other conversions from FBX

        assimpMat.Get(AI_MATKEY_TWOSIDED, modelMat.twoSided);

        aiColor4D baseColor;
        if (assimpMat.Get(AI_MATKEY_BASE_COLOR, baseColor) == AI_SUCCESS)
        {
            modelMat.baseColor = AssimpUtils::convert(baseColor);
        }
        else if (assimpMat.Get(AI_MATKEY_COLOR_DIFFUSE, baseColor) == AI_SUCCESS)
        {
            modelMat.baseColor = AssimpUtils::convert(baseColor);
        }
        aiColor4D specularColor;
        if (assimpMat.Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == AI_SUCCESS)
        {
            modelMat.specularColor = AssimpUtils::convert(specularColor);
        }
        ai_real v;
        if (assimpMat.Get(AI_MATKEY_METALLIC_FACTOR, v) == AI_SUCCESS)
        {
            modelMat.metallicFactor = glm::clamp(v, 0.0f, 1.0f);
        }
        if (assimpMat.Get(AI_MATKEY_SHININESS, v) == AI_SUCCESS)
        {
            modelMat.shininess = v;
        }
        if (assimpMat.Get(AI_MATKEY_ROUGHNESS_FACTOR, v) == AI_SUCCESS)
        {
            modelMat.roughnessFactor = glm::clamp(v, 0.0f, 1.0f);
        }
        if (assimpMat.Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0), v) == AI_SUCCESS)
        {
            modelMat.normalScale = v;
        }
        if (assimpMat.Get(AI_MATKEY_GLTF_TEXTURE_STRENGTH(aiTextureType_LIGHTMAP, 0), v) == AI_SUCCESS)
        {
            modelMat.occlusionStrength = glm::clamp(v, 0.0f, 1.0f);
        }
        aiColor3D emissiveColor;
        if (assimpMat.Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor) == AI_SUCCESS)
        {
            modelMat.emissiveColor = AssimpUtils::convert(emissiveColor);
        }

        if (_config.opacity)
        {
            modelMat.opacityType = _config.opacity.value();
        }
        else
        {
            aiString aiAlphaMode;
            if (assimpMat.Get(AI_MATKEY_GLTF_ALPHAMODE, aiAlphaMode) == AI_SUCCESS)
            {
                std::string alphaMode = aiAlphaMode.C_Str();
                if (alphaMode == "OPAQUE")
                {
                    modelMat.opacityType = OpacityType::Opaque;
                }
                else if (alphaMode == "MASK")
                {
                    modelMat.opacityType = OpacityType::Mask;
                }
                else
                {
                    modelMat.opacityType = OpacityType::Transparent;
                }
            }
            else
            {
                int blendMode = aiBlendMode_Default;
                if (assimpMat.Get(AI_MATKEY_BLEND_FUNC, blendMode) == AI_SUCCESS)
                {
                    switch (blendMode)
                    {
                    case aiBlendMode_Additive:
                        modelMat.opacityType = OpacityType::Transparent;
                        break;
                    case aiBlendMode_Default:
                        modelMat.opacityType = OpacityType::Opaque;
                        break;
                    }
                }
            }
        }
    }

    struct AssimpCalcTangentsOperation final
    {
    public:
        AssimpCalcTangentsOperation() noexcept
        {
            _iface.m_getNumFaces = getNumFaces;
            _iface.m_getNumVerticesOfFace = getNumFaceVertices;
            _iface.m_getNormal = getNormal;
            _iface.m_getPosition = getPosition;
            _iface.m_getTexCoord = getTexCoords;
            _iface.m_setTSpaceBasic = setTangent;

            _context.m_pInterface = &_iface;
        }

        std::vector<glm::vec3> operator()(const aiMesh& mesh) noexcept
        {
            _mesh = mesh;
            _tangents.clear();
            _tangents.resize(mesh.mNumVertices);
            _context.m_pUserData = this;
            genTangSpaceDefault(&_context);
            _mesh.reset();
            return _tangents;
        }

    private:
        SMikkTSpaceInterface _iface{};
        SMikkTSpaceContext _context{};
        OptionalRef<const aiMesh> _mesh;
        std::vector<glm::vec3> _tangents;

        static const aiMesh& getMeshFromContext(const SMikkTSpaceContext* context) noexcept
        {
            return *static_cast<AssimpCalcTangentsOperation*>(context->m_pUserData)->_mesh;
        }

        static int getVertexIndex(const SMikkTSpaceContext* context, int iFace, int iVert) noexcept
        {
            auto& mesh = getMeshFromContext(context);
            return mesh.mFaces[iFace].mIndices[iVert];
        }

        static int getNumFaces(const SMikkTSpaceContext* context) noexcept
        {
            auto& mesh = getMeshFromContext(context);
            return mesh.mNumFaces;
        }

        static int getNumFaceVertices(const SMikkTSpaceContext* context, int iFace) noexcept
        {
            auto& mesh = getMeshFromContext(context);
            return mesh.mFaces[iFace].mNumIndices;
        }

        static void getPosition(const SMikkTSpaceContext* context, float outpos[], int iFace, int iVert) noexcept
        {
            auto& mesh = getMeshFromContext(context);
            auto index = getVertexIndex(context, iFace, iVert);
            auto& pos = mesh.mVertices[index];
            outpos[0] = pos.x;
            outpos[1] = pos.y;
            outpos[2] = pos.z;
        }

        static void getNormal(const SMikkTSpaceContext* context, float outnormal[], int iFace, int iVert) noexcept
        {
            auto& mesh = getMeshFromContext(context);
            auto index = getVertexIndex(context, iFace, iVert);
            auto& norm = mesh.mNormals[index];
            outnormal[0] = norm.x;
            outnormal[1] = norm.y;
            outnormal[2] = norm.z;
        }

        static void getTexCoords(const SMikkTSpaceContext* context, float outuv[], int iFace, int iVert) noexcept
        {
            auto& mesh = getMeshFromContext(context);
            auto index = getVertexIndex(context, iFace, iVert);
            auto& texCoord = mesh.mTextureCoords[0][index];
            outuv[0] = texCoord.x;
            outuv[1] = texCoord.y;
        }

        static void setTangent(const SMikkTSpaceContext* context, const float tangentu[], float fSign, int iFace, int iVert) noexcept
        {
            auto& op = *static_cast<AssimpCalcTangentsOperation*>(context->m_pUserData);
            auto index = getVertexIndex(context, iFace, iVert);
            auto& tangent = op._tangents[index];
            tangent.x = tangentu[0];
            tangent.y = tangentu[1];
            tangent.z = tangentu[2];
        }
    };

    Data AssimpModelConverter::createVertexData(const aiMesh& assimpMesh, const std::vector<aiBone*>& bones) const noexcept
    {
        auto vertexCount = assimpMesh.mNumVertices;
        VertexDataWriter writer(_config.vertexLayout, vertexCount, _allocator);

        std::vector<glm::vec3> tangents;
        if (assimpMesh.mTangents == nullptr && _config.vertexLayout.has(bgfx::Attrib::Tangent))
        {
            AssimpCalcTangentsOperation op;
            tangents = op(assimpMesh);
        }

        for(size_t i = 0; i < assimpMesh.mNumVertices; ++i)
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
            else if (tangents.size() > i)
            {
                writer.write(bgfx::Attrib::Tangent, i, tangents[i]);
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
            ++i;
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
        for(size_t i = 0; i < assimpMesh.mNumFaces; ++i)
        {
            size += assimpMesh.mFaces[i].mNumIndices;
        }
        std::vector<VertexIndex> indices;
        indices.reserve(size);
        for(size_t i = 0; i < assimpMesh.mNumFaces; ++i)
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
        for (size_t i = 0; i < assimpMesh.mNumBones; ++i)
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
                AssimpUtils::convert(bone->mOffsetMatrix)
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
    const std::string AssimpModelImporterImpl::_programDefinesJsonKey = "programDefines";
    const std::string AssimpModelImporterImpl::_skipMeshesJsonKey = "skipMeshes";
    const std::string AssimpModelImporterImpl::_skipNodesJsonKey = "skipNodes";
    const std::string AssimpModelImporterImpl::_defaultTextureJsonKey = "defaultTexture";
    const std::string AssimpModelImporterImpl::_rootMeshJsonKey = "rootMesh";
    const std::string AssimpModelImporterImpl::_opacityJsonKey = "opacity";

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
        if (json.contains(_programDefinesJsonKey))
        {
            config.programDefines = json[_programDefinesJsonKey];
        }
        if (json.contains(_defaultTextureJsonKey))
        {
            config.defaultTexture = json[_defaultTextureJsonKey];
        }
        if (json.contains(_rootMeshJsonKey))
        {
            config.rootMesh = StringUtils::globToRegex(json[_rootMeshJsonKey]);
        }
        if (json.contains(_opacityJsonKey))
        {
            config.opacity = Material::readOpacity(json[_opacityJsonKey]);
        }

        auto loadRegexList = [&json](const std::string& key, std::vector<std::regex>& value)
        {
            if (!json.contains(key))
            {
                return;
            }
            auto& jsonVal = json[key];
            if (jsonVal.is_array())
            {
                for (auto& elm : jsonVal)
                {
                    value.emplace_back(StringUtils::globToRegex(elm));
                }
            }
            else
            {
                value.emplace_back(StringUtils::globToRegex(jsonVal));
            }
        };
        loadRegexList(_skipMeshesJsonKey, config.skipMeshes);
        loadRegexList(_skipNodesJsonKey, config.skipNodes);

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
        std::filesystem::path outputPath = _currentConfig->outputPath;
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