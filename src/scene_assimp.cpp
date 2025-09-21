#include "detail/scene_assimp.hpp"
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
#include <darmok/transform.hpp>
#include <darmok/glm_serialize.hpp>
#include <darmok/mesh_core.hpp>
#include <darmok/mesh_assimp.hpp>
#include <darmok/skeleton_assimp.hpp>

#include <assimp/vector3.h>
#include <assimp/scene.h>
#include <assimp/GltfMaterial.h>

#include <magic_enum/magic_enum.hpp>

namespace darmok
{
    namespace AssimpUtils
    {
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
    
    AssimpSceneDefinitionLoaderImpl::AssimpSceneDefinitionLoaderImpl(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<ITextureSourceLoader> texLoader) noexcept
        : _dataLoader{ dataLoader }
        , _allocator{ allocator }
        , _texLoader{ texLoader }
    {
    }

    void AssimpSceneDefinitionLoaderImpl::setConfig(const Config& config) noexcept
    {
        _config = config;
    }

    bool AssimpSceneDefinitionLoaderImpl::supports(const std::filesystem::path& path) const noexcept
    {
        return _assimpLoader.supports(path);
    }

    AssimpSceneDefinitionLoaderImpl::Result AssimpSceneDefinitionLoaderImpl::operator()(const std::filesystem::path& path)
    {
        auto dataResult = _dataLoader(path);
        if (!dataResult)
        {
            return unexpected{ dataResult.error() };
        }
        AssimpLoader::Config config;
        config.setPath(path);
        auto sceneResult = _assimpLoader.loadFromMemory(dataResult.value(), config);
		if (!sceneResult)
		{
			return unexpected{ sceneResult.error() };
		}
        auto model = std::make_shared<Model>();
        auto basePath = path.parent_path().string();

        AssimpSceneDefinitionConverter converter{ **sceneResult, *model, basePath, _config, _allocator, _texLoader };
        auto result = converter();
        if(!result )
        {
            return unexpected{ result.error() };
		}
        return model;
    }

    AssimpSceneDefinitionConverter::AssimpSceneDefinitionConverter(const aiScene& assimpScene, Definition& sceneDef, const std::filesystem::path& basePath, const ImportConfig& config,
        bx::AllocatorI& alloc, OptionalRef<ITextureSourceLoader> texLoader, OptionalRef<IProgramSourceLoader> progLoader) noexcept
        : _assimpScene{ assimpScene }
		, _scene{ sceneDef }
        , _basePath{ basePath }
        , _config{ config }
        , _allocator{ alloc }
        , _texLoader{ texLoader }
		, _progLoader{ progLoader }
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

    expected<bool, std::string> AssimpSceneDefinitionConverter::updateMeshes(EntityId entity, const std::regex& regex) noexcept
    {
        auto found = false;
        auto addChild = _assimpScene.mNumMeshes > 1;
        for (int i = 0; i < _assimpScene.mNumMeshes; ++i)
        {
            auto assimpMesh = _assimpScene.mMeshes[i];
            auto name = assimpMesh->mName.C_Str();
            if (!std::regex_match(name, regex))
            {
                continue;
            }
            auto result = addMeshComponents(entity, i, addChild);
            if (!result)
            {
				return unexpected{ result.error() };
            }
            found = true;
        }
        return found;
    }
    expected<void, std::string> AssimpSceneDefinitionConverter::addMeshComponents(EntityId entity, int index, bool addChild) noexcept
    {
        if (index < 0 || index >= _assimpScene.mNumMeshes)
        {
            return unexpected<std::string>{"invalid mesh index"};
        }
        auto assimpMesh = _assimpScene.mMeshes[index];
        auto meshResult = getMesh(index);
        if(!meshResult)
        {
            return unexpected{ meshResult.error() };
		}
        auto matPath = getMaterial(assimpMesh->mMaterialIndex);
        
        if (addChild)
        {
            auto childEntity = _scene.createEntity();
            TransformDefinition trans;
            trans.set_name(AssimpUtils::getString(assimpMesh->mName));
            trans.set_parent(entt::to_integral(entity));
            *trans.mutable_scale() = protobuf::convert(glm::vec3{ 1.f });
            _scene.setComponent(childEntity, trans);
            entity = childEntity;
        }

        RenderableDefinition renderable;
        renderable.set_mesh_path(meshResult.value());
        renderable.set_material_path(matPath);
        _scene.setComponent(entity, renderable);

        auto armPath = getArmature(index);
        if (!armPath.empty())
        {
            SkinnableDefinition skinnable;
            skinnable.set_armature_path(armPath);
            _scene.setComponent(entity, skinnable);
        }

        return {};
    }
    
    expected<void, std::string> AssimpSceneDefinitionConverter::operator()() noexcept
    {
        _scene.setName(AssimpUtils::getString(_assimpScene.mName));
        if (!_config.root_mesh_regex().empty())
        {
            auto entity = _scene.createEntity();
            auto result = updateMeshes(entity, std::regex{ _config.root_mesh_regex() });
            if (!result)
            {
                return unexpected{ result.error() };
			}
            return {};
        }
        auto result = updateNode(*_assimpScene.mRootNode);
        if (!result)
        {
            return unexpected{ result.error() };
        }
        return {};
    }

    expected<EntityId, std::string> AssimpSceneDefinitionConverter::updateNode(const aiNode& assimpNode, EntityId parentEntity) noexcept
    {
        auto name = AssimpUtils::getString(assimpNode.mName);
        if(AssimpUtils::match(name, _config.skip_nodes_regex()))
        {
            return 0;
        }
        auto entity = _scene.createEntity();
        TransformDefinition trans;
        TransformDefinitionWrapper{ trans }
            .setName(name)
            .setParent(parentEntity)
            .setLocalMatrix(AssimpUtils::convert(assimpNode.mTransformation));

        _scene.setComponent(entity, trans);

        for(size_t i = 0; i < assimpNode.mNumMeshes; ++i)
        {
            auto index = assimpNode.mMeshes[i];
            auto meshResult = addMeshComponents(entity, index, true);
            if(!meshResult)
            {
                return unexpected{ meshResult.error() };
			}
        }

        for (size_t i = 0; i < _assimpScene.mNumCameras; ++i)
        {
            auto assimpCam = _assimpScene.mCameras[i];
            if (assimpCam->mName == assimpNode.mName)
            {
                updateCamera(entity, *assimpCam);
                break;
            }
        }
        for (size_t i = 0; i < _assimpScene.mNumLights; ++i)
        {
            auto assimpLight = _assimpScene.mLights[i];
            if (assimpLight->mName == assimpNode.mName)
            {
                updateLight(entity, *assimpLight);
                break;
            }
        }

        for (size_t i = 0; i < assimpNode.mNumChildren; ++i)
        {
            auto result = updateNode(*assimpNode.mChildren[i], entity);
            if(!result)
            {
                return unexpected{ result.error() };
			}
        }

        return entity;
    }

    void AssimpSceneDefinitionConverter::updateCamera(EntityId entity, const aiCamera& assimpCam) noexcept
    {
        aiMatrix4x4 mat;
        assimpCam.GetCameraMatrix(mat);

        if (!mat.IsIdentity())
        {
            TransformDefinition trans;
            TransformDefinitionWrapper{ trans }
                .setParent(entity)
                .setName(AssimpUtils::getString(assimpCam.mName))
                .setLocalMatrix(AssimpUtils::convert(mat));
            entity = _scene.createEntity();
            _scene.setComponent(entity, trans);
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
        
		_scene.setComponent(entity, cam);
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

    void AssimpSceneDefinitionConverter::updateLight(EntityId entity, const aiLight& assimpLight) noexcept
    {
        auto pos = AssimpUtils::convert(assimpLight.mPosition);
        auto mat = glm::translate(glm::mat4{ 1.f }, pos);

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
			_scene.setComponent(entity, light);
        }
        else if (assimpLight.mType == aiLightSource_DIRECTIONAL)
        {
			protobuf::DirectionalLight light;
			light.set_intensity(intensity);
            *light.mutable_color() = pbColor;
            _scene.setComponent(entity, light);

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
            _scene.setComponent(entity, light);
        }
        else if (assimpLight.mType == aiLightSource_AMBIENT)
        {
			protobuf::AmbientLight light;
            intensity = AssimpUtils::getIntensity(assimpLight.mColorAmbient);
            color = AssimpUtils::convert(assimpLight.mColorAmbient * (1.F / intensity));
            pbColor = protobuf::convert(color);
            light.set_intensity(intensity);
            *light.mutable_color() = pbColor;
            _scene.setComponent(entity, light);
        }

        TransformDefinition trans;
        TransformDefinitionWrapper{ trans }
            .setParent(entity)
            .setName(AssimpUtils::getString(assimpLight.mName))
            .setLocalMatrix(mat);

        entity = _scene.createEntity();
        _scene.setComponent(entity, trans);
    }

    std::string AssimpSceneDefinitionConverter::getTexture(const aiMaterial& assimpMat, aiTextureType type, unsigned int index) noexcept
    {
        aiString aiPath{};
        if (assimpMat.GetTexture(type, index, &aiPath) != AI_SUCCESS)
        {
            return {};
        }
        std::string path{ aiPath.C_Str() };
        loadTexture(path);
        return path;
    }

    bool AssimpSceneDefinitionConverter::loadTexture(const std::string& path) noexcept
    {
        if (_texturePaths.contains(path))
        {
            return true;
        }

        TextureSource texSrc;
        auto assimpTex = _assimpScene.GetEmbeddedTexture(path.c_str());
        if (assimpTex)
        {
            size_t size = 0;
            if (assimpTex->mHeight == 0)
            {
                // compressed;
                size = assimpTex->mWidth;
            }
            else
            {
                size = static_cast<size_t>(assimpTex->mWidth * assimpTex->mHeight * 4);
            }
            auto encoding = Image::readEncoding(assimpTex->achFormatHint);
            if (encoding == ImageEncoding::Count)
            {
                encoding = Image::getEncodingForPath(path);
            }
            DataView data{ assimpTex->pcData, size };
            auto name = AssimpUtils::getString(assimpTex->mFilename);
            if (name.empty())
            {
                name = path;
            }
            texSrc.set_name(name);
            auto loadResult = TextureSourceWrapper{ texSrc }.loadData(data, encoding);
            if (!loadResult)
            {
                return false;
            }
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
            texSrc = *result.value();
        }
        else
        {
            return false;
        }
        _texturePaths.insert(path);
		_scene.addAsset(path, texSrc);
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

    void AssimpSceneDefinitionConverter::updateMaterial(MaterialDefinition& matDef, const aiMaterial& assimpMat) noexcept
    {
        matDef.set_name(AssimpUtils::getString(assimpMat.GetName()));

        *matDef.mutable_program() = _config.program();
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
            auto texPath = getTexture(assimpMat, elm.assimpType, elm.assimpIndex);
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
            if (loadTexture(texPath))
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
        aiColor3D specularColor;
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

    expected<void, std::string> AssimpSceneDefinitionConverter::updateMesh(MeshSource& meshSrc, const aiMesh& assimpMesh) noexcept
    {
        const std::string name = AssimpUtils::getString(assimpMesh.mName);
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
            return {};
        }

        *meshSrc.mutable_program() = _config.program();

        AssimpMeshSourceConverter converter{ assimpMesh, *meshSrc.mutable_data() };
        auto convertResult = converter();
        if(!convertResult)
        {
            return unexpected{ convertResult.error() };
		}
		
        return {};
    }

    void AssimpSceneDefinitionConverter::updateArmature(ArmatureDefinition& armDef, const aiMesh& assimpMesh) noexcept
    {
        AssimpArmatureDefinitionConverter convert{ assimpMesh, armDef };
		(void)convert();
    }
    
    expected<std::string, std::string> AssimpSceneDefinitionConverter::getMesh(int index) noexcept
    {
        if (index < 0 || index >= _assimpScene.mNumMeshes)
        {
            return {};
        }
        const aiMesh* assimpMesh = _assimpScene.mMeshes[index];
        auto itr = _meshPaths.find(assimpMesh);
        if (itr != _meshPaths.end())
        {
            return itr->second;
        }
        MeshSource meshSrc;
        auto result = updateMesh(meshSrc, *assimpMesh);
        if(!result)
        {
            return unexpected{ result.error() };
		}
        auto path = "mesh_" + std::to_string(index);
        if (meshSrc.name().empty())
        {
            meshSrc.set_name(path);
        }
        _meshPaths.emplace(assimpMesh, path);
		_scene.addAsset(path, meshSrc);
        return path;
    }

    std::string AssimpSceneDefinitionConverter::getArmature(int index) noexcept
    {
        if (index < 0 || index >= _assimpScene.mNumMeshes)
        {
            return {};
        }
        const aiMesh* assimpMesh = _assimpScene.mMeshes[index];
        auto itr = _armaturePaths.find(assimpMesh);
        if (itr != _armaturePaths.end())
        {
            return itr->second;
        }
        ArmatureDefinition armDef;
        updateArmature(armDef, *assimpMesh);
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
        _scene.addAsset(path, armDef);
        return path;
    }

    std::string AssimpSceneDefinitionConverter::getMaterial(int index) noexcept
    {
        if(index < 0 || index >= _assimpScene.mNumMaterials)
        {
            return {};
		}
		const aiMaterial* assimpMat = _assimpScene.mMaterials[index];
        auto itr = _materialPaths.find(assimpMat);
        if (itr != _materialPaths.end())
        {
            return itr->second;
        }
        MaterialDefinition matDef;
        updateMaterial(matDef, *assimpMat);
        auto path = "material_" + std::to_string(index);
        if(matDef.name().empty())
        {
            matDef.set_name(path);
		}
        _materialPaths.emplace(assimpMat, path);
		_scene.addAsset(path, matDef);
        return path;
    }

    AssimpSceneDefinitionLoader::AssimpSceneDefinitionLoader(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<ITextureSourceLoader> texLoader) noexcept
        : _impl{ std::make_unique<AssimpSceneDefinitionLoaderImpl>(dataLoader, allocator, texLoader) }
    {
    }

    AssimpSceneDefinitionLoader::~AssimpSceneDefinitionLoader() noexcept = default;

    AssimpSceneDefinitionLoader& AssimpSceneDefinitionLoader::setConfig(const Config& config) noexcept
    {
        _impl->setConfig(config);
        return *this;
    }

    bool AssimpSceneDefinitionLoader::supports(const std::filesystem::path& path) const noexcept
    {
        return _impl->supports(path);
    }

    AssimpSceneDefinitionLoader::Result AssimpSceneDefinitionLoader::operator()(std::filesystem::path path)noexcept
    {
        return (*_impl)(path);
    }

    AssimpSceneFileImporterImpl::AssimpSceneFileImporterImpl(bx::AllocatorI& alloc)
        : _dataLoader{ alloc }
        , _texLoader{ _dataLoader }
        , _alloc{ alloc }
        , _progLoader{ _dataLoader }
    {
    }

    void AssimpSceneFileImporterImpl::setLogOutput(OptionalRef<std::ostream> log) noexcept
    {
        _defaultCompilerConfig.progCompiler.log = log;
    }

    void AssimpSceneFileImporterImpl::setShadercPath(const std::filesystem::path& path) noexcept
    {
        _defaultCompilerConfig.progCompiler.shadercPath = path;
    }

    void AssimpSceneFileImporterImpl::addIncludePath(const std::filesystem::path& path) noexcept
    {
        _defaultCompilerConfig.progCompiler.includePaths.insert(path);
    }

    void AssimpSceneFileImporterImpl::loadConfig(const nlohmann::ordered_json& json, const std::filesystem::path& basePath, Config& config)
    {
        auto itr = json.find("programPath");
        auto& progRef = *config.mutable_program();
        if (itr != json.end())
        {
            progRef.set_path(itr->get<std::string>());
        }
        itr = json.find("program");
        if (itr != json.end())
        {
            auto val = itr->get<std::string>();
            StandardProgramLoader::Type standard;
            if (Program::Standard::Type_Parse(val, &standard))
            {
                progRef.set_standard(standard);
            }
            else
            {
                progRef.set_path(val);
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

        config.set_compile(true);
        itr = json.find("programCompilerConfig");
        if (itr != json.end())
        {
            _compilerConfig->progCompiler.read(*itr, basePath);
        }
        itr = json.find("compile");
        if (itr != json.end())
        {
            config.set_compile(*itr);
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

    expected<protobuf::VertexLayout, std::string> AssimpSceneFileImporterImpl::loadVertexLayout(const nlohmann::ordered_json& json)
    {
        protobuf::VertexLayout layout;
        expected<void, std::string> result;
        if (json.is_string())
        {
			result = protobuf::read(layout, json.get<std::filesystem::path>());
        }
        else
        {
            result = VertexLayoutWrapper{ layout }.read(json);
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
        _compilerConfig = _defaultCompilerConfig;

        auto& config = _currentConfig.emplace();
        nlohmann::json configJson = input.config;
        if (!input.dirConfig.empty())
        {
            configJson.update(input.dirConfig);
        }
        loadConfig(configJson, input.basePath, config);

        _outputPath.clear();
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
        OptionalRef<ITextureSourceLoader> texLoader = _texLoader;
        if (!_currentConfig->embed_textures())
        {
            texLoader = nullptr;
        }
        _dataLoader.addBasePath(input.basePath);
        try
        {
            AssimpSceneDefinitionConverter converter{ *_currentScene, def, basePath, *_currentConfig, _alloc, texLoader };
            converter.setConfig(input.config);
            auto convertResult = converter();
            if (!convertResult)
            {
                throw std::runtime_error{ "failed to convert scene: " + convertResult.error() };
            }
            if (_currentConfig->compile())
            {
                if (!_compilerConfig)
                {
                    throw std::runtime_error{ "compiler config missing" };
                }
                SceneDefinitionCompiler compiler{ *_compilerConfig, _progLoader };
                auto compileResult = compiler(def);
                if (!compileResult)
                {
                    throw std::runtime_error{ "failed to compile scene: " + compileResult.error() };
                }
            }
        }
        catch (...)
        {
            _dataLoader.removeBasePath(input.basePath);
            throw;
        }
        
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

    AssimpSceneFileImporter::~AssimpSceneFileImporter() = default;

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

    AssimpSceneFileImporter& AssimpSceneFileImporter::setShadercPath(const std::filesystem::path& path) noexcept
    {
        _impl->setShadercPath(path);
        return *this;
    }

    AssimpSceneFileImporter& AssimpSceneFileImporter::addIncludePath(const std::filesystem::path& path) noexcept
    {
        _impl->addIncludePath(path);
        return *this;
    }

}