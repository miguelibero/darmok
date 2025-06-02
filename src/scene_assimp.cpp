#include "scene_assimp.hpp"
#include <filesystem>
#include <fstream>
#include <map>

#include <darmok/scene_assimp.hpp>
#include <darmok/image.hpp>
#include <darmok/data.hpp>
#include <darmok/vertex.hpp>
#include <darmok/program_core.hpp>
#include <darmok/string.hpp>
#include <darmok/math.hpp>
#include <darmok/material.hpp>
#include <darmok/glm_serialize.hpp>

#include <assimp/vector3.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/GltfMaterial.h>
#include <assimp/BaseImporter.h>

#include <mikktspace.h>
#include <magic_enum/magic_enum.hpp>
#include <glm/gtx/component_wise.hpp>

namespace darmok
{
    namespace AssimpUtils
    {
        std::string_view getStringView(const aiString& str) noexcept
        {
            return std::string_view{ str.data, str.length };
        }

        std::string getString(const aiString& str) noexcept
        {
            return std::string{ str.data, str.length };
        }

        glm::mat4 convert(const aiMatrix4x4& from) noexcept
        {
            // the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
            return glm::mat4{
                from.a1, from.b1, from.c1, from.d1,
                from.a2, from.b2, from.c2, from.d2,
                from.a3, from.b3, from.c3, from.d3,
                from.a4, from.b4, from.c4, from.d4
            };
        }

        glm::vec3 convert(const aiVector3D& vec) noexcept
        {
            return glm::vec3{ vec.x, vec.y, vec.z };
        }

        glm::vec2 convert(const aiVector2D& vec) noexcept
        {
            return glm::vec2{ vec.x, vec.y };
        }

        uint8_t convertColorComp(ai_real v) noexcept
        {
            return 255 * v;
        }

        Color convert(const aiColor4D& c) noexcept
        {
            return Color
            {
                convertColorComp(c.r),
                convertColorComp(c.g),
                convertColorComp(c.b),
                convertColorComp(c.a),
            };
        }

        float getIntensity(const aiColor3D& c) noexcept
        {
            return glm::compMax(glm::vec3{ c.r, c.g, c.b });
        }

        Color3 convert(aiColor3D c) noexcept
        {
            return Color3
            {
                convertColorComp(c.r),
                convertColorComp(c.g),
                convertColorComp(c.b)
            };
        }

        bool fixImportConfig(IDataLoader& dataLoader, protobuf::AssimpSceneImportConfig& config)
        {
			auto stride = VaryingUtils::getBgfx(config.vertex_layout()).getStride();
            if (stride == 0)
            {
                if (config.has_standard_program())
                {
                    auto def = StandardProgramLoader::loadDefinition(config.standard_program());
                    *config.mutable_vertex_layout() = def->varying().vertex();
                }
                else if (config.program_path().empty())
                {
                    protobuf::Program prog;
                    auto result = protobuf::read(prog, config.program_path());
                    if (!result)
                    {
                        return false;
                    }
                    *config.mutable_vertex_layout() = prog.varying().vertex();
                }
            }
            return stride > 0;
        }

		bool match(const std::string& str, const google::protobuf::RepeatedPtrField<std::string>& regexes) noexcept
		{
			for (const auto& regex : regexes)
			{
                if (std::regex_match(str, std::regex{ regex }))
				{
					return true;
				}
			}
            return false;
		}
    };

    void AssimpLoader::Config::setPath(const std::filesystem::path& path) noexcept
    {
        basePath = path.parent_path().string();
        format = path.extension().string();
    }

    bool AssimpLoader::supports(const std::filesystem::path& path) const noexcept
    {
        Assimp::Importer importer;
        return importer.IsExtensionSupported(path.extension().string());
    }

    unsigned int AssimpLoader::getImporterFlags(const Config& config) noexcept
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

    AssimpLoader::Result AssimpLoader::loadFromFile(const std::filesystem::path& path, const Config& config) const
    {
        Assimp::Importer importer;
        const aiScene* ptr = nullptr;
        if (!config.format.empty())
        {
            auto hintImporter = importer.GetImporter(config.format.c_str());
            if (hintImporter != nullptr)
            {
                ptr = hintImporter->ReadFile(&importer, path.string().c_str(), importer.GetIOHandler());
            }
        }
        if(ptr == nullptr)
        {
            ptr = importer.ReadFile(path.string(), getImporterFlags(config));
        }
        if (ptr == nullptr)
        {
            return unexpected{ importer.GetErrorString() };
        }
        return fixScene(importer);
    }

    AssimpLoader::Result AssimpLoader::loadFromMemory(const DataView& data, const Config& config) const
    {
        Assimp::Importer importer;
        if (!config.basePath.empty())
        {
            auto path = std::filesystem::path(config.basePath) / "file";
            if (!config.format.empty())
            {
                path += "." + config.format;
            }
            importer.SetPropertyString("sourceFilePath", path.string());
        }

        auto ptr = importer.ReadFileFromMemory(data.ptr(), data.size(), getImporterFlags(config), config.format.c_str());
        if (ptr == nullptr)
        {
            return unexpected{ importer.GetErrorString() };
        }
        return fixScene(importer);
    }

    std::shared_ptr<aiScene> AssimpLoader::fixScene(Assimp::Importer& importer) noexcept
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

    AssimpSceneDefinitionLoaderImpl::AssimpSceneDefinitionLoaderImpl(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<ITextureDefinitionLoader> texLoader) noexcept
        : _dataLoader{ dataLoader }
        , _allocator{ allocator }
        , _texLoader{ texLoader }
    {
    }

    void AssimpSceneDefinitionLoaderImpl::setConfig(const Config& config) noexcept
    {
        _config = config;
        AssimpUtils::fixImportConfig(_dataLoader, _config);
    }

    bool AssimpSceneDefinitionLoaderImpl::supports(const std::filesystem::path& path) const noexcept
    {
        return _assimpLoader.supports(path);
    }

    AssimpSceneDefinitionLoaderImpl::Result AssimpSceneDefinitionLoaderImpl::operator()(const std::filesystem::path& path)
    {
        AssimpLoader::Config config;
        config.setPath(path);
        auto dataResult = _dataLoader(path);
        if (!dataResult)
        {
            return unexpected{ dataResult.error() };
        }
        auto sceneResult = _assimpLoader.loadFromMemory(dataResult.value(), config);
		if (!sceneResult)
		{
			return unexpected{ sceneResult.error() };
		}
        auto model = std::make_shared<Model>();
        auto basePath = path.parent_path().string();

        AssimpSceneDefinitionConverter converter{ **sceneResult, basePath, _config, _allocator, _texLoader };
        converter(*model);
        return model;
    }

    AssimpSceneDefinitionConverter::AssimpSceneDefinitionConverter(const aiScene& scene, const std::filesystem::path& basePath, const ImportConfig& config,
        bx::AllocatorI& alloc, OptionalRef<ITextureDefinitionLoader> texLoader) noexcept
        : _scene{ scene }
        , _basePath{ basePath }
        , _config{ config }
        , _allocator{ alloc }
        , _texLoader{ texLoader }
    {
    }

    std::vector<std::string> AssimpSceneDefinitionConverter::getTexturePaths(const aiScene& scene) noexcept
    {
        std::vector<std::string> paths;
        for (size_t i = 0; i < scene.mNumMaterials; ++i)
        {
            auto& assimpMat = *scene.mMaterials[i];
            for (auto& elm : _materialTextures)
            {
                aiString path{ "" };
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

    AssimpSceneDefinitionConverter& AssimpSceneDefinitionConverter::setBoneNames(const std::vector<std::string>& names) noexcept
    {
        _boneNames.clear();
        for (auto& name : names)
        {
            _boneNames.emplace(name, name);
        }
        return *this;
    }

    AssimpSceneDefinitionConverter& AssimpSceneDefinitionConverter::setBoneNames(const std::unordered_map<std::string, std::string>& names) noexcept
    {
        _boneNames = names;
        return *this;
    }

    AssimpSceneDefinitionConverter& AssimpSceneDefinitionConverter::setConfig(const nlohmann::json& config) noexcept
    {
        auto itr = config.find("bones");
        if (itr != config.end())
        {
            auto& bonesConfig = *itr;
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

    uint32_t AssimpSceneDefinitionConverter::createEntity(Definition& def) noexcept
    {
        auto v = def.registry().entities() + 1;
        def.mutable_registry()->set_entities(v);
        return v;
    }

    bool AssimpSceneDefinitionConverter::addAsset(Definition& def, std::string_view path, Message& asset) noexcept
    {
        auto typeId = protobuf::getTypeId(asset);
        auto& assetPack = *def.mutable_assets();
        auto& assets = assetPack.mutable_groups()->try_emplace(typeId).first->second;
        auto result = assets.mutable_assets()->try_emplace(path);
		result.first->second.PackFrom(asset);
        return result.second;
    }

    bool AssimpSceneDefinitionConverter::addComponent(Definition& def, uint32_t entityId, Message& comp) noexcept
    {
        auto typeId = protobuf::getTypeId(comp);
        auto& components = def.mutable_registry()->mutable_components()->try_emplace(typeId).first->second;
        auto result = components.mutable_components()->try_emplace(entityId);
		result.first->second.PackFrom(comp);
        return result.second;
    }

    bool AssimpSceneDefinitionConverter::updateMeshes(Definition& def, uint32_t entityId, const std::regex& regex) noexcept
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
            if (addMeshComponents(def, entityId, i))
            {
                found = true;
            }
        }
        return found;
    }
    bool AssimpSceneDefinitionConverter::addMeshComponents(Definition& def, uint32_t entityId, int index) noexcept
    {
        if (index < 0 || index >= _scene.mNumMeshes)
        {
            return false;
        }
        auto assimpMesh = _scene.mMeshes[index];
        auto meshPath = getMesh(def, index);
        auto matPath = getMaterial(def, assimpMesh->mMaterialIndex);

        auto childEntityId = createEntity(def);
        TransformDefinition trans;
        trans.set_name(AssimpUtils::getString(assimpMesh->mName));
        trans.set_parent(entityId);
        addComponent(def, childEntityId, trans);

        RenderableDefinition renderable;
        renderable.set_mesh_path(meshPath);
        renderable.set_material_path(matPath);
        addComponent(def, childEntityId, renderable);

        auto armPath = getArmature(def, index);
        if (!armPath.empty())
        {
            SkinnableDefinition skinnable;
            skinnable.set_armature_path(armPath);
            addComponent(def, childEntityId, skinnable);
        }

        return true;
    }

    bool AssimpSceneDefinitionConverter::operator()(Definition& def) noexcept
    {
        def.set_name(AssimpUtils::getString(_scene.mName));

        if (!_config.root_mesh_regex().empty())
        {
            auto entityId = createEntity(def);
            return updateMeshes(def, entityId, std::regex{ _config.root_mesh_regex() });
        }
        return updateNode(def, *_scene.mRootNode).has_value();
    }

    std::optional<uint32_t> AssimpSceneDefinitionConverter::updateNode(Definition& def, const aiNode& assimpNode, uint32_t parentEntityId) noexcept
    {
        auto name = AssimpUtils::getString(assimpNode.mName);
        if(AssimpUtils::match(name, _config.skip_nodes_regex()))
        {
            return std::nullopt;
        }

        auto entityId = createEntity(def);
        TransformDefinition trans;
        trans.set_name(name);
        trans.set_parent(parentEntityId);
        *trans.mutable_matrix() = protobuf::convert(
            AssimpUtils::convert(assimpNode.mTransformation)
        );
        addComponent(def, entityId, trans);

        for(size_t i = 0; i < assimpNode.mNumMeshes; ++i)
        {
            auto index = assimpNode.mMeshes[i];
            addMeshComponents(def, entityId, index);
        }

        for (size_t i = 0; i < _scene.mNumCameras; ++i)
        {
            auto assimpCam = _scene.mCameras[i];
            if (assimpCam->mName == assimpNode.mName)
            {
                updateCamera(def, entityId, *assimpCam);
                break;
            }
        }
        for (size_t i = 0; i < _scene.mNumLights; ++i)
        {
            auto assimpLight = _scene.mLights[i];
            if (assimpLight->mName == assimpNode.mName)
            {
                updateLight(def, entityId, *assimpLight);
                break;
            }
        }

        for (size_t i = 0; i < assimpNode.mNumChildren; ++i)
        {
            if (!updateNode(def, *assimpNode.mChildren[i], entityId))
            {
                continue;
            }
        }

        return entityId;
    }

    void AssimpSceneDefinitionConverter::updateCamera(Definition& def, uint32_t entityId, const aiCamera& assimpCam) noexcept
    {
        aiMatrix4x4 mat;
        assimpCam.GetCameraMatrix(mat);

        if (!mat.IsIdentity())
        {
            TransformDefinition trans;
            trans.set_parent(entityId);
            trans.set_name(AssimpUtils::getString(assimpCam.mName));
            *trans.mutable_matrix() = protobuf::convert(AssimpUtils::convert(mat));
            entityId = createEntity(def);
            addComponent(def, entityId, trans);
        }
        CameraDefinition cam;
		cam.set_near(assimpCam.mClipPlaneNear);
        cam.set_far(assimpCam.mClipPlaneFar);
        if (assimpCam.mOrthographicWidth == 0.0f)
        {
            auto aspect = assimpCam.mAspect;
            auto fovy = 0.f;
            if (aspect != 0.f)
            {
                auto fovx = assimpCam.mHorizontalFOV;
                fovy = 2.f * atan(tan(0.5f * fovx) * aspect);
            }
			cam.set_perspective_fovy(fovy);
        }
        else
        {
            float w = assimpCam.mOrthographicWidth;
            float h = w / assimpCam.mAspect;

            auto forward = assimpCam.mLookAt;
            auto right = forward ^ assimpCam.mUp;
            right.Normalize();
            auto up = right ^ forward;
            up.Normalize();

            auto center = assimpCam.mPosition + forward * assimpCam.mClipPlaneNear;
            auto bottomLeft = center - (right * (w / 2.0f)) - (up * (h / 2.0f));
            auto offset = center - bottomLeft;

            auto& uv = *cam.mutable_ortho_center();
            uv.set_x((offset * right) / w);
            uv.set_y((offset * up) / h);            
        }
        
		addComponent(def, entityId, cam);
    }

    float AssimpSceneDefinitionConverter::getLightRange(const glm::vec3& attenuation) noexcept
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

    void AssimpSceneDefinitionConverter::updateLight(Definition& def, uint32_t entityId, const aiLight& assimpLight) noexcept
    {
        TransformDefinition trans;
        trans.set_parent(entityId);
        trans.set_name(AssimpUtils::getString(assimpLight.mName));

        auto pos = AssimpUtils::convert(assimpLight.mPosition);
        auto mat = glm::translate(glm::mat4(1), pos);

        auto intensity = AssimpUtils::getIntensity(assimpLight.mColorDiffuse);
        auto color = AssimpUtils::convert(assimpLight.mColorDiffuse * (1.F / intensity));
		auto pbColor = protobuf::convert(color);

        if (assimpLight.mType == aiLightSource_POINT)
        {
            protobuf::PointLight light;
            auto attn = glm::vec3(
                assimpLight.mAttenuationConstant,
                assimpLight.mAttenuationLinear,
                assimpLight.mAttenuationQuadratic
            );
            light.set_range(getLightRange(attn));
            light.set_intensity(intensity);
            *light.mutable_color() = pbColor;
            // we're not supporting different specular color in lights
			addComponent(def, entityId, light);
        }
        else if (assimpLight.mType == aiLightSource_DIRECTIONAL)
        {
			protobuf::DirectionalLight light;
			light.set_intensity(intensity);
            *light.mutable_color() = pbColor;
            addComponent(def, entityId, light);

            auto dir = AssimpUtils::convert(assimpLight.mDirection);
            glm::mat4 view = glm::lookAt(pos, pos + dir, glm::vec3{0, 1, 0});
            mat *= glm::inverse(view);
        }
        else if (assimpLight.mType == aiLightSource_SPOT)
        {
			protobuf::SpotLight light;
            light.set_intensity(intensity);
            *light.mutable_color() = pbColor;
			light.set_cone_angle(assimpLight.mAngleOuterCone);
			light.set_inner_cone_angle(assimpLight.mAngleInnerCone);
            addComponent(def, entityId, light);
        }
        else if (assimpLight.mType == aiLightSource_AMBIENT)
        {
			protobuf::AmbientLight light;
            intensity = AssimpUtils::getIntensity(assimpLight.mColorAmbient);
            color = AssimpUtils::convert(assimpLight.mColorAmbient * (1.F / intensity));
            pbColor = protobuf::convert(color);
            light.set_intensity(intensity);
            *light.mutable_color() = pbColor;
            addComponent(def, entityId, light);
        }

        *trans.mutable_matrix() = protobuf::convert(mat);
        entityId = createEntity(def);
        addComponent(def, entityId, trans);
    }

    std::string AssimpSceneDefinitionConverter::getTexture(Definition& def, const aiMaterial& assimpMat, aiTextureType type, unsigned int index) noexcept
    {
        aiString aiPath{};
        if (assimpMat.GetTexture(type, index, &aiPath) != AI_SUCCESS)
        {
            return {};
        }
        std::string path{ aiPath.C_Str() };
        loadTexture(def, path);
        return path;
    }

    bool AssimpSceneDefinitionConverter::loadTexture(Definition& def, const std::string& path) noexcept
    {
        if (_texturePaths.contains(path))
        {
            return true;
        }

        TextureDefinition texDef;
        auto assimpTex = _scene.GetEmbeddedTexture(path.c_str());
        if (assimpTex)
        {
            size_t size = 0;
            auto format = bimg::TextureFormat::Count;
            if (assimpTex->mHeight == 0)
            {
                // compressed;
                size = assimpTex->mWidth;
            }
            else
            {
                size = static_cast<size_t>(assimpTex->mWidth * assimpTex->mHeight * 4);
                format = bimg::TextureFormat::RGBA8;
            }
            DataView data{ assimpTex->pcData, size };
            texDef.set_name(path);
            // TODO: can we read the texture config?
            TextureUtils::loadImage(texDef, Image{ data, _allocator, format });

        }
        else if (_texLoader)
        {
            std::filesystem::path fsPath{ path };
            if (fsPath.is_relative())
            {
                fsPath = _basePath / fsPath;
            }
            auto result = (*_texLoader)(fsPath.string());
            if (!result)
            {
                return false;
            }
            texDef = *result.value();
        }
        else
        {
            return false;
        }
        _texturePaths.insert(path);
		addAsset(def, path, texDef);
        return true;
    }

    const std::vector<AssimpSceneDefinitionConverter::AssimpMaterialTexture> AssimpSceneDefinitionConverter::_materialTextures =
    {        
        { AI_MATKEY_BASE_COLOR_TEXTURE, Material::TextureDefinition::BaseColor },
        { aiTextureType_DIFFUSE, 0, Material::TextureDefinition::BaseColor },
        { aiTextureType_SPECULAR, 0, Material::TextureDefinition::Specular },
        { AI_MATKEY_METALLIC_TEXTURE, Material::TextureDefinition::MetallicRoughness },
        { AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, Material::TextureDefinition::MetallicRoughness },
        { aiTextureType_NORMALS, 0, Material::TextureDefinition::Normal },
        { aiTextureType_AMBIENT_OCCLUSION, 0, Material::TextureDefinition::Occlusion },
        { aiTextureType_LIGHTMAP, 0, Material::TextureDefinition::Occlusion },
        { aiTextureType_EMISSIVE, 0, Material::TextureDefinition::Emissive },
    };

    void AssimpSceneDefinitionConverter::updateMaterial(Definition& def, MaterialDefinition& matDef, const aiMaterial& assimpMat) noexcept
    {
        matDef.set_name(AssimpUtils::getString(assimpMat.GetName()));

        if (_config.has_standard_program())
        {
            matDef.set_standard_program(_config.standard_program());
        }
        else
        {
            matDef.set_program_path(_config.program_path());
        }        
        for (auto& define : _config.program_defines())
        {
			auto itr = std::find(matDef.program_defines().begin(), matDef.program_defines().end(), define);
            if (itr == matDef.program_defines().end())
            {
                *matDef.mutable_program_defines()->Add() = define;
            }
        }

        auto& textures = *matDef.mutable_textures();
        for (auto& elm : _materialTextures)
        {
            auto texPath = getTexture(def, assimpMat, elm.assimpType, elm.assimpIndex);
            if (!texPath.empty())
            {
                auto& matTex = *textures.Add();
                matTex.set_type(elm.darmokType);
				matTex.set_texture_path(texPath);
            }
        }

		auto itr = std::find_if(textures.begin(), textures.end(), [](auto& tex)
		{
			return tex.type() == Material::TextureDefinition::BaseColor;
		});

        if (itr == textures.end() && !_config.default_texture_path().empty())
        {
            auto texPath = _config.default_texture_path();
            if (loadTexture(def, texPath))
            {
				auto& matTex = *textures.Add();
				matTex.set_type(Material::TextureDefinition::BaseColor);
				matTex.set_texture_path(texPath);
            }
        }

        // TODO: convert aiTextureType_METALNESS + aiTextureType_DIFFUSE_ROUGHNESS
        // TODO: also other conversions from FBX

		bool twoSided = false;
        if (assimpMat.Get(AI_MATKEY_TWOSIDED, twoSided))
        {
			matDef.set_twosided(twoSided);
        }

        aiColor4D baseColor;
        if (assimpMat.Get(AI_MATKEY_BASE_COLOR, baseColor) == AI_SUCCESS)
        {
            *matDef.mutable_base_color() = protobuf::convert(AssimpUtils::convert(baseColor));
        }
        else if (assimpMat.Get(AI_MATKEY_COLOR_DIFFUSE, baseColor) == AI_SUCCESS)
        {
            *matDef.mutable_base_color() = protobuf::convert(AssimpUtils::convert(baseColor));
        }
        aiColor4D specularColor;
        if (assimpMat.Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == AI_SUCCESS)
        {
            *matDef.mutable_specular_color() = protobuf::convert(AssimpUtils::convert(specularColor));
        }
        ai_real v = 0;
        if (assimpMat.Get(AI_MATKEY_METALLIC_FACTOR, v) == AI_SUCCESS)
        {
            matDef.set_metallic_factor(glm::clamp(v, 0.0f, 1.0f));
        }
        if (assimpMat.Get(AI_MATKEY_SHININESS, v) == AI_SUCCESS)
        {
            matDef.set_shininess(v);
        }
        if (assimpMat.Get(AI_MATKEY_ROUGHNESS_FACTOR, v) == AI_SUCCESS)
        {
            matDef.set_roughness_factor(glm::clamp(v, 0.0f, 1.0f));
        }
        if (assimpMat.Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0), v) == AI_SUCCESS)
        {
            matDef.set_normal_scale(v);
        }
        if (assimpMat.Get(AI_MATKEY_GLTF_TEXTURE_STRENGTH(aiTextureType_LIGHTMAP, 0), v) == AI_SUCCESS)
        {
            matDef.set_occlusion_strength(glm::clamp(v, 0.0f, 1.0f));
        }
        aiColor3D emissiveColor;
        if (assimpMat.Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor) == AI_SUCCESS)
        {
            *matDef.mutable_emissive_color() = protobuf::convert(AssimpUtils::convert(emissiveColor));
        }

        if (_config.has_opacity())
        {
            matDef.set_opacity_type(_config.opacity());
        }
        else
        {
            aiString aiAlphaMode;
            if (assimpMat.Get(AI_MATKEY_GLTF_ALPHAMODE, aiAlphaMode) == AI_SUCCESS)
            {
                std::string alphaMode = aiAlphaMode.C_Str();
                if (alphaMode == "OPAQUE")
                {
                    matDef.set_opacity_type(Material::Definition::Opaque);
                }
                else if (alphaMode == "MASK")
                {
                    matDef.set_opacity_type(Material::Definition::Mask);
                }
                else
                {
                    matDef.set_opacity_type(Material::Definition::Transparent);
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
                        matDef.set_opacity_type(Material::Definition::Transparent);
                        break;
                    case aiBlendMode_Default:
                        matDef.set_opacity_type(Material::Definition::Opaque);
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

    std::string AssimpSceneDefinitionConverter::createVertexData(const aiMesh& assimpMesh, const std::vector<aiBone*>& bones) const noexcept
    {
        auto vertexCount = assimpMesh.mNumVertices;
        auto layout = VaryingUtils::getBgfx(_config.vertex_layout());
        VertexDataWriter writer(layout, vertexCount, _allocator);

        std::vector<glm::vec3> tangents;
        if (assimpMesh.mTangents == nullptr && layout.has(bgfx::Attrib::Tangent))
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
        return writer.finish().toString();
    }

    bool AssimpSceneDefinitionConverter::updateBoneData(const std::vector<aiBone*>& bones, VertexDataWriter& writer) const noexcept
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
            glm::vec4 weights{ 1, 0, 0, 0 };
            glm::vec4 indices{ -1 };
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

    std::string AssimpSceneDefinitionConverter::createIndexData(const aiMesh& assimpMesh) const noexcept
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
        return std::string(reinterpret_cast<std::string::value_type*>(indices.data()),
            size * sizeof(VertexIndex) / sizeof(std::string::value_type));
    }

    void AssimpSceneDefinitionConverter::updateMesh(Definition& def, MeshDefinition& meshDef, const aiMesh& assimpMesh) noexcept
    {
        meshDef.set_name(AssimpUtils::getString(assimpMesh.mName));
        auto& bounds = *meshDef.mutable_bounds();
		*bounds.mutable_min() = protobuf::convert(AssimpUtils::convert(assimpMesh.mAABB.mMin));
        *bounds.mutable_max() = protobuf::convert(AssimpUtils::convert(assimpMesh.mAABB.mMax));
            
        const std::string name(assimpMesh.mName.C_Str());
        auto skip = false;
        for (auto& regex : _skipMeshes)
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
        bones.reserve(assimpMesh.mNumBones);
        for (size_t i = 0; i < assimpMesh.mNumBones; ++i)
        {
            bones.push_back(assimpMesh.mBones[i]);
        }

        meshDef.set_vertices(createVertexData(assimpMesh, bones));
        meshDef.set_indices(createIndexData(assimpMesh));
        *meshDef.mutable_layout() = _config.vertex_layout();
    }

    void AssimpSceneDefinitionConverter::updateArmature(Definition& def, ArmatureDefinition& armDef, const aiMesh& assimpMesh) noexcept
    {
        for (size_t i = 0; i < assimpMesh.mNumBones; ++i)
        {
            auto bone = assimpMesh.mBones[i];
            auto boneName = AssimpUtils::getString(bone->mName);
            if (!_boneNames.empty())
            {
                auto itr = std::find_if(_boneNames.begin(), _boneNames.end(),
                    [&boneName](auto& elm) { return elm.first == boneName; });
                if (itr == _boneNames.end())
                {
                    continue;
                }
                boneName = itr->second;
            }
            auto& joint = *armDef.add_joints();
            joint.set_name(boneName);
            *joint.mutable_inverse_bind_pose() = protobuf::convert(AssimpUtils::convert(bone->mOffsetMatrix));
        }
    }
    
    std::string AssimpSceneDefinitionConverter::getMesh(Definition& def, int index) noexcept
    {
        if (index < 0 || index >= _scene.mNumMeshes)
        {
            return {};
        }
        const aiMesh* assimpMesh = _scene.mMeshes[index];
        auto itr = _meshPaths.find(assimpMesh);
        if (itr != _meshPaths.end())
        {
            return itr->second;
        }
        MeshDefinition meshDef;
        updateMesh(def, meshDef, *assimpMesh);
        auto path = "mesh_" + std::to_string(index);
        if (meshDef.name().empty())
        {
            meshDef.set_name(path);
        }
        _meshPaths.emplace(assimpMesh, path);
		addAsset(def, path, meshDef);
        return path;
    }

    std::string AssimpSceneDefinitionConverter::getArmature(Definition& def, int index) noexcept
    {
        if (index < 0 || index >= _scene.mNumMeshes)
        {
            return {};
        }
        const aiMesh* assimpMesh = _scene.mMeshes[index];
        auto itr = _armaturePaths.find(assimpMesh);
        if (itr != _armaturePaths.end())
        {
            return itr->second;
        }
        ArmatureDefinition armDef;
        updateArmature(def, armDef, *assimpMesh);
        if (armDef.joints_size() == 0)
        {
            return {};
        }
        auto path = "armature_" + std::to_string(index);
        if (armDef.name().empty())
        {
            armDef.set_name(path);
        }
        _armaturePaths.emplace(assimpMesh, path);
        addAsset(def, path, armDef);
        return path;
    }

    std::string AssimpSceneDefinitionConverter::getMaterial(Definition& def, int index) noexcept
    {
        if(index < 0 || index >= _scene.mNumMaterials)
        {
            return {};
		}
		const aiMaterial* assimpMat = _scene.mMaterials[index];
        auto itr = _materialPaths.find(assimpMat);
        if (itr != _materialPaths.end())
        {
            return itr->second;
        }
        MaterialDefinition matDef;
        updateMaterial(def, matDef, *assimpMat);
        auto path = "material_" + std::to_string(index);
        if(matDef.name().empty())
        {
            matDef.set_name(path);
		}
        _materialPaths.emplace(assimpMat, path);
		addAsset(def, path, matDef);
        return path;
    }

    AssimpSceneDefinitionLoader::AssimpSceneDefinitionLoader(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<ITextureDefinitionLoader> texLoader) noexcept
        : _impl{ std::make_unique<AssimpSceneDefinitionLoaderImpl>(dataLoader, allocator, texLoader) }
    {
    }

    AssimpSceneDefinitionLoader::~AssimpSceneDefinitionLoader() noexcept
    {
        // empty to forward declare the impl pointer
    }

    AssimpSceneDefinitionLoader& AssimpSceneDefinitionLoader::setConfig(const Config& config) noexcept
    {
        _impl->setConfig(config);
        return *this;
    }

    bool AssimpSceneDefinitionLoader::supports(const std::filesystem::path& path) const noexcept
    {
        return _impl->supports(path);
    }

    AssimpSceneDefinitionLoader::Result AssimpSceneDefinitionLoader::operator()(std::filesystem::path path)
    {
        return (*_impl)(path);
    }

    AssimpSceneFileImporterImpl::AssimpSceneFileImporterImpl(bx::AllocatorI& alloc)
        : _dataLoader{ alloc }
        , _imgLoader{ _dataLoader, alloc }
        , _texLoader{ _imgLoader }
        , _alloc{ alloc }
        , _progLoader{ _dataLoader }
    {
    }

    void AssimpSceneFileImporterImpl::loadConfig(const nlohmann::ordered_json& json, const std::filesystem::path& basePath, Config& config)
    {
        // TODO: maybe load material json?
        auto itr = json.find("vertexLayout");
        if (itr != json.end())
        {
            auto layoutResult = loadVertexLayout(*itr);
            *config.mutable_vertex_layout() = layoutResult.value();
        }
        itr = json.find("programPath");
        if (itr != json.end())
        {
            auto programPath = basePath / *itr;
            config.set_program_path(programPath.string());
            protobuf::ProgramSource src;
			auto result = protobuf::read(src, programPath);
            if (!result)
            {
                throw std::runtime_error("failed to load vertex layout from program path");
            }
            *config.mutable_vertex_layout() = src.varying().vertex();
        }
        itr = json.find("program");
        if (itr != json.end())
        {
            auto val = itr->get<std::string>();
            StandardProgramLoader::Type standard;            
            if (Program::Standard::Type_Parse(val, &standard))
            {
                config.set_standard_program(standard);
            }
            else
            {
                config.set_program_path(val);
            }
        }
        itr = json.find("programDefines");
        if (itr != json.end())
        {
            for (auto& elm : *itr)
            {
                config.add_program_defines(elm.get<std::string>());
            }
        }
        itr = json.find("defaultTexture");
        if (itr != json.end())
        {
            config.set_default_texture_path(itr->get<std::string>());
        }
        itr = json.find("rootMesh");
        if (itr != json.end())
        {
            auto val = itr->get<std::string>();
            config.set_root_mesh_regex(StringUtils::globToRegex(val));
        }
        itr = json.find("opacity");
        if (itr != json.end())
        {
            auto val = itr->get<std::string>();
            Material::OpacityType opacity;
            if (Material::Definition::OpacityType_Parse(val, &opacity))
            {
                config.set_opacity(opacity);
            }
        }
        auto addRegexes = [](auto& repeated, const auto& json)
        {
            if (json.is_array())
            {
                for (auto& elm : json)
                {
                    repeated.Add(StringUtils::globToRegex(elm));
                }
            }
            else
            {
                repeated.Add(StringUtils::globToRegex(json));
            }
        };
        itr = json.find("skipMeshes");
		if (itr != json.end())
		{
			addRegexes(*config.mutable_skip_meshes_regex(), *itr);

		}
        itr = json.find("skipNodes");
        if (itr != json.end())
        {
            addRegexes(*config.mutable_skip_nodes_regex(), *itr);
        }
        itr = json.find("embedTextures");
        if (itr != json.end())
        {
            config.set_embed_textures(*itr);
        }
        itr = json.find("shadowType");
        if (itr != json.end())
        {
            auto val = itr->get<std::string>();
            static const std::string suffix = "Shadow";
            if (!StringUtils::endsWith(val, suffix))
            {
                val += suffix;
            }
            protobuf::Light::ShadowType shadowType;
            if(protobuf::Light::ShadowType_Parse(val, &shadowType))
            {
                config.set_shadow_type(shadowType);
            }
        }
    }

    expected<VertexLayout, std::string> AssimpSceneFileImporterImpl::loadVertexLayout(const nlohmann::ordered_json& json)
    {
        VertexLayout layout;
        expected<void, std::string> result;
        if (json.is_string())
        {
			result = protobuf::read(layout, json.get<std::filesystem::path>());
        }
        else
        {
            result = VaryingUtils::read(layout, json);
        }
        if (!result)
        {
            return unexpected(result.error());
        }
        return layout;
    }

    bool AssimpSceneFileImporterImpl::startImport(const Input& input, bool dry)
    {
        if (input.config.is_null())
        {
            return false;
        }
        if (!_assimpLoader.supports(input.path))
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
        AssimpUtils::fixImportConfig(_dataLoader, config);

        auto itr = configJson.find("outputPath");
        if (itr != configJson.end())
        {
            _outputPath = itr->get<std::filesystem::path>();
        }
        itr = configJson.find("outputFormat");
        if (itr != configJson.end())
        {
            std::string_view val{ *itr };
            _outputFormat = protobuf::getFormat(val).value();
        }
        else if (!_outputPath.empty())
        {
            _outputFormat = protobuf::getFormat(_outputPath);
        }

        AssimpLoader::Config sceneConfig;
        {
            auto itr = input.config.find("format");
            if (itr != input.config.end())
            {
                sceneConfig.format = *itr;
            }
            itr = input.config.find("loadPath");
            if (itr != input.config.end())
            {
                sceneConfig.basePath = *itr;
            }
        }
        auto result = _assimpLoader.loadFromFile(input.path, sceneConfig);
		if (!result)
		{
			return false;
		}
        _currentScene = *result;
        return _currentScene != nullptr;
    }

    void AssimpSceneFileImporterImpl::endImport(const Input& input)
    {
        _currentConfig.reset();
        _currentScene.reset();
    }

    std::vector<std::filesystem::path> AssimpSceneFileImporterImpl::getOutputs(const Input& input)
    {
        std::vector<std::filesystem::path> outputs;
        if (_outputPath.empty())
        {
            const std::string stem = input.path.stem().string();
            _outputPath = stem + std::string{ protobuf::getExtension(_outputFormat) };
        }
        auto basePath = input.getRelativePath().parent_path();
        outputs.push_back(basePath / _outputPath);
        return outputs;
    }

    AssimpSceneFileImporterImpl::Dependencies AssimpSceneFileImporterImpl::getDependencies(const Input& input)
    {
        Dependencies deps;
        if (!_currentConfig || !_currentConfig->embed_textures())
        {
            return deps;
        }
        auto basePath = input.getRelativePath().parent_path();
        for (auto& texPath : AssimpSceneDefinitionConverter::getTexturePaths(*_currentScene))
        {
            deps.insert(basePath / texPath);
        }
        return deps;
    }

    std::ofstream AssimpSceneFileImporterImpl::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path) const
    {
        return protobuf::createOutputStream(path, _outputFormat);
    }

    void AssimpSceneFileImporterImpl::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        if (!_currentConfig)
        {
            return;
        }
        Definition def;
        auto basePath = input.getRelativePath().parent_path();
        OptionalRef<ITextureDefinitionLoader> texLoader = _texLoader;
        if (!_currentConfig->embed_textures())
        {
            texLoader = nullptr;
        }
        _dataLoader.addBasePath(input.basePath);
        AssimpSceneDefinitionConverter converter{ *_currentScene, basePath, *_currentConfig, _alloc, texLoader };
        converter.setConfig(input.config);
        converter(def);
        _dataLoader.removeBasePath(input.basePath);
		auto writeResult = protobuf::write(def, out, _outputFormat);
        if(!writeResult)
		{
            throw std::runtime_error{ "failed to write output: " + writeResult.error() };
		}
    }

    const std::string& AssimpSceneFileImporterImpl::getName() const noexcept
    {
        static const std::string name{ "scene" };
        return name;
    }

    AssimpSceneFileImporter::AssimpSceneFileImporter(bx::AllocatorI& alloc)
        : _impl{ std::make_unique<AssimpSceneFileImporterImpl>(alloc) }
    {
    }

    AssimpSceneFileImporter::~AssimpSceneFileImporter()
    {
        // empty on purpose
    }

    bool AssimpSceneFileImporter::startImport(const Input& input, bool dry)
    {
        return _impl->startImport(input, dry);
    }

    std::vector<std::filesystem::path> AssimpSceneFileImporter::getOutputs(const Input& input)
    {
        return _impl->getOutputs(input);
    }

    AssimpSceneFileImporter::Dependencies AssimpSceneFileImporter::getDependencies(const Input& input)
    {
        return _impl->getDependencies(input);
    }

    std::ofstream AssimpSceneFileImporter::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
    {
        return _impl->createOutputStream(input, outputIndex, path);
    }

    void AssimpSceneFileImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        return _impl->writeOutput(input, outputIndex, out);
    }

    void AssimpSceneFileImporter::endImport(const Input& input)
    {
        return _impl->endImport(input);
    }

    const std::string& AssimpSceneFileImporter::getName() const noexcept
    {
        return _impl->getName();
    }
}