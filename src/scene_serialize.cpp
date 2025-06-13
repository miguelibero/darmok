#include <darmok/scene_serialize.hpp>
#include <darmok/scene.hpp>
#include "scene_serialize.hpp"

#include <darmok/camera.hpp>
#include <darmok/transform.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/light.hpp>

namespace darmok
{
    ConstSceneDefinitionWrapper::ConstSceneDefinitionWrapper(const Definition& def) noexcept
    : _def{ def }
    {
    }

    const ConstSceneDefinitionWrapper::Definition& ConstSceneDefinitionWrapper::get() const noexcept
    {
        return *_def;
    }

    std::vector<uint32_t> ConstSceneDefinitionWrapper::getRootEntities() const noexcept
    {
        std::vector<uint32_t> entities;
        for (auto& [entityId, trans] : getTypeComponents<Transform::Definition>())
        {
            if (trans.parent() == 0)
            {
                entities.push_back(entityId);
            }
        }
        return entities;
    }

    OptionalRef<const ConstSceneDefinitionWrapper::RegistryComponents> ConstSceneDefinitionWrapper::getTypeComponents(uint32_t typeId) const noexcept
    {
        auto& comps = _def->registry().components();
        auto itr = comps.find(typeId);
        if (itr == comps.end())
        {
            return std::nullopt;
        }
        return itr->second;
    }

    OptionalRef<const ConstSceneDefinitionWrapper::AssetGroup> ConstSceneDefinitionWrapper::getAssetGroup(uint32_t typeId) const noexcept
    {
        auto& assetGroups = _def->assets().groups();
        auto itr = assetGroups.find(typeId);
        if (itr == assetGroups.end())
        {
            return std::nullopt;
        }
        return itr->second;
    }

    std::vector<std::string> ConstSceneDefinitionWrapper::getAssetPaths(uint32_t typeId) const noexcept
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

    SceneDefinitionWrapper::SceneDefinitionWrapper(Definition& def) noexcept
    : ConstSceneDefinitionWrapper(def)
    , _def{ &def }
    {
    }

    SceneDefinitionWrapper::Definition& SceneDefinitionWrapper::get() noexcept
    {
        return *_def;
    }

    uint32_t SceneDefinitionWrapper::createEntity() noexcept
    {
        auto& reg = *_def->mutable_registry();
        auto v = reg.entities() + 1;
        reg.set_entities(v);
        return v;
    }

    bool SceneDefinitionWrapper::addAsset(std::string_view path, Message& asset) noexcept
    {
        auto typeId = protobuf::getTypeId(asset);
        auto& assetPack = *_def->mutable_assets();
        auto& assets = assetPack.mutable_groups()->try_emplace(typeId).first->second;
        auto result = assets.mutable_assets()->try_emplace(path);
        result.first->second.PackFrom(asset);
        return result.second;
    }

    bool SceneDefinitionWrapper::addComponent(uint32_t entityId, Message& comp) noexcept
    {
        auto typeId = protobuf::getTypeId(comp);
        auto& components = _def->mutable_registry()->mutable_components()->try_emplace(typeId).first->second;
        auto result = components.mutable_components()->try_emplace(entityId);
        result.first->second.PackFrom(comp);
        return result.second;
    }

    std::vector<std::reference_wrapper<SceneDefinitionWrapper::Any>> SceneDefinitionWrapper::getComponents(uint32_t entityId) noexcept
    {
        std::vector<std::reference_wrapper<Any>> comps;
        for(auto& [typeId, typeComps] : *_def->mutable_registry()->mutable_components())
        {
            auto& components = *typeComps.mutable_components();
            auto itr = components.find(entityId);
            if (itr != components.end())
            {
                comps.push_back(std::ref(itr->second));
            }
		}
        return comps;
    }

    OptionalRef<SceneDefinitionWrapper::RegistryComponents> SceneDefinitionWrapper::getTypeComponents(uint32_t typeId) noexcept
    {
        auto& comps = *_def->mutable_registry()->mutable_components();
        auto itr = comps.find(typeId);
        if (itr == comps.end())
        {
            return std::nullopt;
        }
        return itr->second;
    }

    OptionalRef<SceneDefinitionWrapper::Any> SceneDefinitionWrapper::getComponent(uint32_t entityId, uint32_t typeId) noexcept
    {
		auto typeComps = getTypeComponents(typeId);
        if(!typeComps)
        {
            return std::nullopt;
        }
        auto& components = *typeComps->mutable_components();
        auto itr = components.find(entityId);
        if (itr == components.end())
        {
            return std::nullopt;
		}
		return itr->second;
    }

    OptionalRef<SceneDefinitionWrapper::AssetGroup> SceneDefinitionWrapper::getAssetGroup(uint32_t typeId) noexcept
    {
        auto& assetGroups = *_def->mutable_assets()->mutable_groups();
        auto itr = assetGroups.find(typeId);
        if (itr == assetGroups.end())
        {
            return std::nullopt;
        }
        return itr->second;
    }

    std::unordered_map<std::string, OptionalRef<SceneDefinitionWrapper::Any>> SceneDefinitionWrapper::getAssets(uint32_t typeId) noexcept
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

    OptionalRef<SceneDefinitionWrapper::Any> SceneDefinitionWrapper::getAsset(uint32_t typeId, const std::string& path)
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

    SceneArchive::SceneArchive(const protobuf::Scene& sceneDef, Scene& scene, const AssetPackFallbacks& assetPackFallbacks)
        : _sceneDef{ sceneDef }
        , _scene{ scene }
		, _assetPack{ sceneDef.assets(), assetPackFallbacks }
        , _count{ 0 }
        , _type{ 0 }
		, _entityOffset{ 0 }
    {
        auto view = scene.getRegistry().view<Entity>();
        if (view.begin() != view.end())
        {
            auto max = *std::max_element(view.begin(), view.end());
            _entityOffset = static_cast<entt::id_type>(max) + 1;
        }
    }

    entt::continuous_loader SceneArchive::createLoader()
    {
		return { _scene.getRegistry() };
    }

    void SceneArchive::operator()(std::underlying_type_t<Entity>& count)
    {
        if (_type == 0)
        {
            count = _sceneDef.registry().entities();
        }
        else
        {
            auto& typeComps = _sceneDef.registry().components();
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
		return _assetPack;
    }

    Entity SceneArchive::getEntity(uint32_t id) const
    {
		return static_cast<Entity>(id + _entityOffset);
    }

    const Scene& SceneArchive::getScene() const
    {
        return _scene;
    }

    Scene& SceneArchive::getScene()
    {
        return _scene;
    }

    expected<Entity, std::string> SceneArchive::finishLoad()
    {
        expected<void, std::string> result;
        for (auto& func : _compLoadFunctions)
        {
            auto funcResult = func();
            if (!funcResult)
            {
                result = std::move(funcResult);
			}
        }
        _compLoadFunctions.clear();
        if (!result)
        {
			return unexpected{ result.error() };
        }

        for(uint32_t id = 0; id < _sceneDef.registry().entities(); ++id)
        {
            auto entity = getEntity(id);
            if (auto trans = _scene.getComponent<Transform>(entity))
            {
                if (!trans->getParent())
                {
                    return entity;
                }
            }
		}
        return unexpected{ "could not find root entity" };
    }

    SceneImporterImpl::SceneImporterImpl(const AssetPackFallbacks& assetPackFallbacks)
        : _assetPackFallbacks{ assetPackFallbacks }
    {
	}

    SceneImporterImpl::Result SceneImporterImpl::operator()(Scene& scene, const Definition& def)
    {
		SceneArchive archive{ def, scene, _assetPackFallbacks };

		archive.registerComponent<Transform>();
        archive.registerComponent<Renderable>();
        archive.registerComponent<Camera>();
        archive.registerComponent<Skinnable>();
        archive.registerComponent<PointLight>();
        archive.registerComponent<DirectionalLight>();
        archive.registerComponent<SpotLight>();
        archive.registerComponent<AmbientLight>();

		auto loader = archive.createLoader();

        loader.get<entt::entity>(archive);
        auto result = archive.loadComponents(loader);
        if (!result)
        {
            return unexpected{ result.error() };
        }
        loader.orphans();
        return archive.finishLoad();
    }

    SceneImporter::SceneImporter(const AssetPackFallbacks& assetPackFallbacks)
        : _impl(std::make_unique<SceneImporterImpl>(assetPackFallbacks))
    {
    }

    SceneImporter::~SceneImporter()
    {
        // empty on purpose
    }

    SceneImporter::Result SceneImporter::operator()(Scene& scene, const Definition& def)
    {
		return (*_impl)(scene, def);
    }

    SceneLoader::SceneLoader(ISceneDefinitionLoader& defLoader, const AssetPackFallbacks& assetPackFallbacks)
        : _defLoader{ defLoader }
        , _importer{ assetPackFallbacks }
	{
	}

    SceneLoader::Result SceneLoader::operator()(Scene& scene, std::filesystem::path path)
    {
		auto defResult = _defLoader(path);
        if (!defResult)
        {
			return unexpected<Error>{ defResult.error() };
        }
		return _importer(scene, **defResult);
    }
}