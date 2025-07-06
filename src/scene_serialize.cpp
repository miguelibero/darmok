#include <darmok/scene_serialize.hpp>
#include <darmok/scene.hpp>
#include "scene_serialize.hpp"

#include <darmok/camera.hpp>
#include <darmok/transform.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/light.hpp>

#include <fmt/format.h>

namespace darmok
{
    ConstSceneDefinitionWrapper::ConstSceneDefinitionWrapper(const Definition& def) noexcept
    : _def{ def }
    {
    }

    const std::string& ConstSceneDefinitionWrapper::getName() const noexcept
    {
		return _def->name();
    }

    std::vector<Entity> ConstSceneDefinitionWrapper::getRootEntities() const noexcept
    {
        std::vector<Entity> entities;
        for (auto& [entity, trans] : getTypeComponents<Transform::Definition>())
        {
            if (!trans.has_parent() || trans.parent() == 0)
            {
                entities.push_back(entity);
            }
        }
        return entities;
    }

    std::vector<Entity> ConstSceneDefinitionWrapper::getChildren(Entity entity) const noexcept
    {
        std::vector<Entity> entities;
        for (auto& [childEntity, trans] : getTypeComponents<Transform::Definition>())
        {
            if (trans.parent() == entt::to_integral(entity))
            {
                entities.push_back(childEntity);
            }
        }
        return entities;
    }

    OptionalRef<const ConstSceneDefinitionWrapper::RegistryComponents> ConstSceneDefinitionWrapper::getTypeComponents(IdType typeId) const noexcept
    {
        auto& comps = _def->registry().components();
        auto itr = comps.find(typeId);
        if (itr == comps.end())
        {
            return std::nullopt;
        }
        return itr->second;
    }

    OptionalRef<const ConstSceneDefinitionWrapper::Any> ConstSceneDefinitionWrapper::getComponent(Entity entity, IdType typeId) const noexcept
    {
        auto typeComps = getTypeComponents(typeId);
        if (!typeComps)
        {
            return std::nullopt;
        }
        auto& components = typeComps->components();
        auto itr = components.find(entt::to_integral(entity));
        if (itr == components.end())
        {
            return std::nullopt;
        }
        return itr->second;
    }

    std::optional<Entity> ConstSceneDefinitionWrapper::getEntity(const Message& comp) const noexcept
    {
        auto typeId = protobuf::getTypeId(comp);
        auto typeComps = getTypeComponents(typeId);
        if (!typeComps)
        {
            return std::nullopt;
        }
        Any anyComp;
        if (!anyComp.PackFrom(comp))
        {
            return std::nullopt;
        }
        for (auto& [entity, any] : typeComps->components())
        {
            if (any.SerializeAsString() == anyComp.SerializeAsString())
            {
                return Entity{ entity };
            }
        }
		return std::nullopt;
    }

    OptionalRef<const ConstSceneDefinitionWrapper::AssetGroup> ConstSceneDefinitionWrapper::getAssetGroup(IdType typeId) const noexcept
    {
        auto& assetGroups = _def->assets().groups();
        auto itr = assetGroups.find(typeId);
        if (itr == assetGroups.end())
        {
            return std::nullopt;
        }
        return itr->second;
    }

    std::vector<std::string> ConstSceneDefinitionWrapper::getAssetPaths(IdType typeId) const noexcept
    {
        std::vector<std::string> paths;
        auto group = getAssetGroup(typeId);
        if (!group)
        {
            return paths;
        }
        paths.reserve(group->assets_size());
        for (auto& [path, asset] : group->assets())
        {
            paths.push_back(path);
        }
        return paths;
    }


    std::optional<std::string> ConstSceneDefinitionWrapper::getAssetPath(const Message& asset) const noexcept
    {
        auto typeId = protobuf::getTypeId(asset);
        auto group = getAssetGroup(typeId);
        if (!group)
        {
            return std::nullopt;
        }
        Any anyAsset;
        if (!anyAsset.PackFrom(asset))
        {
            return std::nullopt;
        }
        for (const auto& [path, any] : group->assets())
        {
            if (any.SerializeAsString() == anyAsset.SerializeAsString())
            {
                return path;
            }
        }
        return std::nullopt;
    }

    SceneDefinitionWrapper::SceneDefinitionWrapper(Definition& def) noexcept
    : ConstSceneDefinitionWrapper(def)
    , _def{ &def }
    {
    }

    void SceneDefinitionWrapper::setName(std::string_view name) noexcept
    {
        _def->set_name(std::string{ name });
    }

    Entity SceneDefinitionWrapper::createEntity() noexcept
    {
        auto& reg = *_def->mutable_registry();
        auto v = reg.entities() + 1;
        reg.set_entities(v);
        return static_cast<Entity>(v);
    }

    bool SceneDefinitionWrapper::setAsset(std::string_view path, const Message& asset) noexcept
    {
        auto typeId = protobuf::getTypeId(asset);
        auto& assetPack = *_def->mutable_assets();
        auto& assets = assetPack.mutable_groups()->try_emplace(typeId).first->second;
        auto result = assets.mutable_assets()->try_emplace(path);
        if(protobuf::isAny(asset))
        {
			result.first->second = static_cast<const Any&>(asset);
		}
        else
        {
            result.first->second.PackFrom(asset);
        }
        return result.second;
    }


    std::string SceneDefinitionWrapper::addAsset(std::string_view pathPrefix, const Message& asset) noexcept
    {
        auto typeId = protobuf::getTypeId(asset);
        auto& assetPack = *_def->mutable_assets();
        auto& assets = *assetPack.mutable_groups()->try_emplace(typeId).first->second.mutable_assets();
		std::string path{ pathPrefix };
        auto itr = assets.find(path);
        auto i = 0;
        while (itr != assets.end())
        {
            path = fmt::format("{}_{}", pathPrefix, ++i);
            itr = assets.find(path);
        }
        if (protobuf::isAny(asset))
        {
            assets.emplace(path, static_cast<const Any&>(asset));
        }
        else
        {
            assets.emplace(path, Any{}).first->second.PackFrom(asset);
        }

        return path;
    }

    bool SceneDefinitionWrapper::setComponent(Entity entity, const Message& comp) noexcept
    {
        auto typeId = protobuf::getTypeId(comp);
        auto& components = _def->mutable_registry()->mutable_components()->try_emplace(typeId).first->second;
        auto result = components.mutable_components()->try_emplace(entt::to_integral(entity));
        if (protobuf::isAny(comp))
        {
            result.first->second = static_cast<const Any&>(comp);
        }
        else
        {
            result.first->second.PackFrom(comp);
        }
        return result.second;
    }

    std::vector<std::reference_wrapper<SceneDefinitionWrapper::Any>> SceneDefinitionWrapper::getComponents(Entity entity) noexcept
    {
        std::vector<std::reference_wrapper<Any>> comps;
        for(auto& [typeId, typeComps] : *_def->mutable_registry()->mutable_components())
        {
            auto& components = *typeComps.mutable_components();
            auto itr = components.find(entt::to_integral(entity));
            if (itr != components.end())
            {
                comps.push_back(std::ref(itr->second));
            }
		}
        return comps;
    }

    OptionalRef<SceneDefinitionWrapper::RegistryComponents> SceneDefinitionWrapper::getTypeComponents(IdType typeId) noexcept
    {
        auto& comps = *_def->mutable_registry()->mutable_components();
        auto itr = comps.find(typeId);
        if (itr == comps.end())
        {
            return std::nullopt;
        }
        return itr->second;
    }

    OptionalRef<SceneDefinitionWrapper::Any> SceneDefinitionWrapper::getComponent(Entity entity, IdType typeId) noexcept
    {
		auto typeComps = getTypeComponents(typeId);
        if(!typeComps)
        {
            return std::nullopt;
        }
        auto& components = *typeComps->mutable_components();
        auto itr = components.find(entt::to_integral(entity));
        if (itr == components.end())
        {
            return std::nullopt;
		}
		return itr->second;
    }

    bool SceneDefinitionWrapper::destroyEntity(Entity entity) noexcept
    {
        bool found = false;
        for (auto& [typeId, typeComps] : *_def->mutable_registry()->mutable_components())
        {
            auto& components = *typeComps.mutable_components();
            auto itr = components.find(entt::to_integral(entity));
            if (itr != components.end())
            {
                components.erase(itr);
                found = true;
            }
        }
        return found;
    }

    bool SceneDefinitionWrapper::removeComponent(Entity entity, IdType typeId) noexcept
    {
        auto typeComps = getTypeComponents(typeId);
        if (!typeComps)
        {
            return false;
        }
        return typeComps->mutable_components()->erase(typeId) > 0;
    }

    bool SceneDefinitionWrapper::removeComponent(const Message& comp) noexcept
    {
        auto typeId = protobuf::getTypeId(comp);
        auto typeComps = getTypeComponents(typeId);
        if (!typeComps)
        {
            return false;
        }
        auto& comps = *typeComps->mutable_components();
        auto itr = std::find_if(comps.begin(), comps.end(),
            [&comp](const auto& pair) {
                return &pair.second == &comp;
			});
        if (itr == comps.end())
        {
            return false;
        }
        comps.erase(itr);
		return true;
    }

    OptionalRef<SceneDefinitionWrapper::AssetGroup> SceneDefinitionWrapper::getAssetGroup(IdType typeId) noexcept
    {
        auto& assetGroups = *_def->mutable_assets()->mutable_groups();
        auto itr = assetGroups.find(typeId);
        if (itr == assetGroups.end())
        {
            return std::nullopt;
        }
        return itr->second;
    }

    std::unordered_map<std::string, OptionalRef<SceneDefinitionWrapper::Any>> SceneDefinitionWrapper::getAssets(IdType typeId) noexcept
    {
        std::unordered_map<std::string, OptionalRef<Any>> assets;
        auto group = getAssetGroup(typeId);
        if (!group)
        {
            return assets;
        }
        assets.reserve(group->assets_size());
        for (auto& [path, asset] : *group->mutable_assets())
        {
            assets[path] = asset;
        }
        return assets;
    }

    OptionalRef<SceneDefinitionWrapper::Any> SceneDefinitionWrapper::getAsset(IdType typeId, const std::string& path)
    {
        auto group = getAssetGroup(typeId);
        if (!group)
        {
            return std::nullopt;
        }
        auto& assets = *group->mutable_assets();
        auto itr = assets.find(path);
        if (itr == assets.end())
        {
            return std::nullopt;
        }
        return itr->second;
    }

    bool SceneDefinitionWrapper::removeAsset(IdType typeId, const std::string& path) noexcept
    {
        auto group = getAssetGroup(typeId);
        if (!group)
        {
            return false;
        }
        return group->mutable_assets()->erase(path) > 0;
    }

    bool SceneDefinitionWrapper::removeAsset(const Message& asset) noexcept
    {
        auto typeId = protobuf::getTypeId(asset);
        auto group = getAssetGroup(typeId);
        if (!group)
        {
            return false;
        }
        auto& assets = *group->mutable_assets();
        auto itr = std::find_if(assets.begin(), assets.end(),
            [&asset](const auto& pair) {
                return &pair.second == &asset;
            });
        if (itr == assets.end())
        {
            return false;
        }
        assets.erase(itr);
        return true;
    }

    SceneArchive::SceneArchive(Scene& scene, const AssetPackConfig& assetPackConfig)
        : _loader{ scene.getRegistry() }
        , _scene{ scene }
		, _assetPackConfig{ assetPackConfig }
        , _count{ 0 }
        , _type{ 0 }
    {
    }

    expected<Entity, std::string> SceneArchive::load(const protobuf::Scene& sceneDef)
    {
		_sceneDef = sceneDef;
        _assetPack.emplace(sceneDef.assets(), _assetPackConfig);
        _loader.get<entt::entity>(*this);
        auto result = loadComponents();
        if (!result)
        {
            return unexpected{ result.error() };
        }
        _loader.orphans();

        result = {};
        for (auto& func : _postLoadFuncs)
        {
            auto funcResult = func();
            if (!funcResult)
            {
                result = std::move(funcResult);
            }
        }
        _postLoadFuncs.clear();
        if (!result)
        {
            return unexpected{ result.error() };
        }

		ConstSceneDefinitionWrapper def{ sceneDef };
		auto roots = def.getRootEntities();
        if(roots.empty())
        {
            return unexpected{ "No root entities found in the scene definition." };
		}
		return getEntity(entt::to_integral(roots.front()));
    }

    void SceneArchive::operator()(std::underlying_type_t<Entity>& count)
    {
        if(!_sceneDef)
        {
            _error = "Scene definition is not loaded.";
            count = 0;
            return;
		}
		auto& reg = _sceneDef->registry();
        if (_type == 0)
        {
            count = reg.entities();
        }
        else
        {
            auto& typeComps = reg.components();
            auto itr = typeComps.find(_type);
            if (itr != typeComps.end())
            {
				count = itr->second.components_size();
            }
        }
		_error.reset();
        _count = 0;
    }

    AssetPack& SceneArchive::getAssets()
    {
		return *_assetPack;
    }

    Entity SceneArchive::getEntity(uint32_t id) const
    {
		return _loader.map(static_cast<Entity>(id));
    }

    const Scene& SceneArchive::getScene() const
    {
        return _scene;
    }

    Scene& SceneArchive::getScene()
    {
        return _scene;
    }

    SceneImporterImpl::SceneImporterImpl(Scene& scene, const AssetPackConfig& assetPackConfig)
        : _archive{ scene, assetPackConfig }
    {
        _archive.registerComponent<Transform>();
        _archive.registerComponent<Renderable>();
        _archive.registerComponent<Camera>();
        _archive.registerComponent<Skinnable>();
        _archive.registerComponent<PointLight>();
        _archive.registerComponent<DirectionalLight>();
        _archive.registerComponent<SpotLight>();
        _archive.registerComponent<AmbientLight>();
	}

    SceneImporterImpl::Result SceneImporterImpl::operator()(const Scene::Definition& def)
    {
        return _archive.load(def);
    }

    SceneImporter::SceneImporter(Scene& scene, const AssetPackConfig& assetPackConfig)
        : _impl{ std::make_unique<SceneImporterImpl>(scene, assetPackConfig) }
    {
    }

    SceneImporter::~SceneImporter()
    {
        // empty on purpose
    }

    SceneImporter::Result SceneImporter::operator()(const Scene::Definition& def)
    {
		return (*_impl)(def);
    }

    SceneLoader::SceneLoader(ISceneDefinitionLoader& defLoader, const AssetPackConfig& assetPackConfig)
        : _defLoader{ defLoader }
        , _assetPackConfig{ assetPackConfig }
	{
	}

    SceneLoader::Result SceneLoader::operator()(Scene& scene, std::filesystem::path path)
    {
		auto defResult = _defLoader(path);
        if (!defResult)
        {
			return unexpected<Error>{ defResult.error() };
        }
        SceneImporter importer{ scene, _assetPackConfig};
		return importer(*defResult.value());
    }
}