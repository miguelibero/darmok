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

    std::optional<Entity> ConstSceneDefinitionWrapper::getEntity(const Any& anyComp) const noexcept
    {
        auto typeId = protobuf::getTypeId(anyComp);
        auto typeComps = getTypeComponents(typeId);
        if (!typeComps)
        {
            return std::nullopt;
        }
		auto& comps = typeComps->components();
        auto itr = std::find_if(comps.begin(), comps.end(),
            [&anyComp](const auto& pair) {
                return &pair.second == &anyComp;
            });
        if (itr != comps.end())
        {
            return Entity{ itr->first };
        }
		return std::nullopt;
    }

    std::vector<std::filesystem::path> ConstSceneDefinitionWrapper::getAssetPaths(IdType typeId) const noexcept
    {
        auto& assetPack = _def->assets();
        std::vector<std::filesystem::path> paths;
        paths.reserve(assetPack.assets_size());
        for (auto& [path, any] : assetPack.assets())
        {
            if(protobuf::getTypeId(any) != typeId)
            {
                continue;
			}
            paths.push_back(path);
        }
        return paths;
    }

    std::optional<std::filesystem::path> ConstSceneDefinitionWrapper::getChildPath(const std::filesystem::path& path, const std::filesystem::path& parentPath) noexcept
    {
        static const char sep = '/';

        auto normalize = [](const std::filesystem::path& path)
        {
            auto str = path.lexically_normal().string();
            StringUtils::replace(str, std::filesystem::path::preferred_separator, sep);
            return str;
        };

        auto pathStr = normalize(path);
        auto parentPathStr = normalize(parentPath);
        if (parentPathStr == ".")
        {
            parentPathStr.clear();
        }

        auto pos = pathStr.find(parentPathStr);
        if (pos != 0)
        {
            return std::nullopt;
        }
        pos += parentPathStr.size();
        auto endPos = pathStr.find(sep, pos);
        return pathStr.substr(pos, endPos);
    }

    std::unordered_map<std::filesystem::path, OptionalRef<const ConstSceneDefinitionWrapper::Any>> ConstSceneDefinitionWrapper::getAssets(IdType typeId) const noexcept
    {
        auto& assetPack = _def->assets();
        std::unordered_map<std::filesystem::path, OptionalRef<const Any>> assets;
        assets.reserve(assetPack.assets_size());
        for (auto& [path, any] : assetPack.assets())
        {
            if (typeId != 0 || protobuf::getTypeId(any) != typeId)
            {
                continue;
            }
            assets[path] = any;
        }
        return assets;
    }

    std::unordered_map<std::filesystem::path, OptionalRef<const ConstSceneDefinitionWrapper::Any>> ConstSceneDefinitionWrapper::getAssets(const std::filesystem::path& parentPath) const noexcept
    {
        auto& assetPack = _def->assets();
        std::unordered_map<std::filesystem::path, OptionalRef<const Any>> assets;
        assets.reserve(assetPack.assets_size());
        for (auto& [pathStr, any] : assetPack.assets())
        {
            std::filesystem::path path{ pathStr };
            if (path.parent_path() == parentPath)
            {
                assets[path] = any;
            }
            else if(auto childPath = getChildPath(path, parentPath))
            {
                assets[*childPath] = nullptr;
			}
        }
        return assets;
    }

    OptionalRef<const ConstSceneDefinitionWrapper::Any> ConstSceneDefinitionWrapper::getAsset(const std::filesystem::path& path) const noexcept
    {
        auto& assetPack = _def->assets();
        auto itr = assetPack.assets().find(path.string());
        if (itr == assetPack.assets().end())
        {
            return std::nullopt;
        }
		return itr->second;
    }

    std::optional<std::filesystem::path> ConstSceneDefinitionWrapper::getAssetPath(const Any& anyAsset) const noexcept
    {
        auto& assetPack = _def->assets();
		auto& assets = assetPack.assets();
        auto itr = std::find_if(assets.begin(), assets.end(),
            [&anyAsset](const auto& pair) {
                return &pair.second == &anyAsset;
			});
        if(itr != assets.end())
        {
            return itr->first;
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

    bool SceneDefinitionWrapper::setAsset(const std::filesystem::path& path, const Message& asset) noexcept
    {
        auto typeId = protobuf::getTypeId(asset);
        auto& assetPack = *_def->mutable_assets();
        auto result = assetPack.mutable_assets()->try_emplace(path.string());
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


    std::filesystem::path SceneDefinitionWrapper::addAsset(const std::filesystem::path& pathPrefix, const Message& asset) noexcept
    {
        auto typeId = protobuf::getTypeId(asset);
        auto& assetPack = *_def->mutable_assets();
        auto& assets = *assetPack.mutable_assets();
		std::string path{ pathPrefix.string() };
        auto itr = assets.find(path);
        auto i = 0;
        while (itr != assets.end())
        {
            path = fmt::format("{}_{}", pathPrefix.string(), ++i);
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

    bool SceneDefinitionWrapper::removeComponent(const Any& anyComp) noexcept
    {
        auto typeId = protobuf::getTypeId(anyComp);
        auto typeComps = getTypeComponents(typeId);
        if (!typeComps)
        {
            return false;
        }
        auto& comps = *typeComps->mutable_components();
        auto itr = std::find_if(comps.begin(), comps.end(),
            [&anyComp](const auto& pair) {
                return &pair.second == &anyComp;
			});
        if (itr == comps.end())
        {
            return false;
        }
        comps.erase(itr);
		return true;
    }

    std::unordered_map<std::filesystem::path, OptionalRef<SceneDefinitionWrapper::Any>> SceneDefinitionWrapper::getAssets(IdType typeId) noexcept
    {
		auto& assetPack = *_def->mutable_assets();
        std::unordered_map<std::filesystem::path, OptionalRef<Any>> assets;
        assets.reserve(assetPack.assets_size());
        for (auto& [path, any] : *assetPack.mutable_assets())
        {
            if(typeId != 0 && protobuf::getTypeId(any) != typeId)
            {
                continue;
			}
            assets[path] = any;
        }
        return assets;
    }

    std::unordered_map<std::filesystem::path, OptionalRef<SceneDefinitionWrapper::Any>> SceneDefinitionWrapper::getAssets(const std::filesystem::path& parentPath) noexcept
    {
        auto ppath = parentPath;
        if (ppath == ".")
        {
            ppath = std::filesystem::path{};
        }
        auto& assetPack = *_def->mutable_assets();
        std::unordered_map<std::filesystem::path, OptionalRef<Any>> assets;
        assets.reserve(assetPack.assets_size());
        for (auto& [pathStr, any] : *assetPack.mutable_assets())
        {
            std::filesystem::path path{ pathStr };
            if (path.parent_path() == ppath)
            {
                assets[path] = any;
            }
			else if (auto childPath = getChildPath(path, ppath))
            {
                assets[*childPath] = nullptr;
            }
        }
        return assets;
    }

    OptionalRef<SceneDefinitionWrapper::Any> SceneDefinitionWrapper::getAsset(const std::filesystem::path& path)
    {
        auto& assets = *_def->mutable_assets()->mutable_assets();
        auto itr = assets.find(path.string());
        if (itr == assets.end())
        {
            return std::nullopt;
        }

        return itr->second;
    }

    bool SceneDefinitionWrapper::removeAsset(const std::filesystem::path& path) noexcept
    {
        return _def->mutable_assets()->mutable_assets()->erase(path.string()) > 0;
    }

    bool SceneDefinitionWrapper::removeAsset(const Any& anyAsset) noexcept
    {
		auto anyStr = anyAsset.SerializeAsString();
        auto& assets = *_def->mutable_assets()->mutable_assets();
        auto itr = std::find_if(assets.begin(), assets.end(),
            [&anyAsset](const auto& pair) {
                return &pair.second == &anyAsset;
            });
        if (itr == assets.end())
        {
            return false;
        }
        assets.erase(itr);
        return true;
    }

    SceneArchive::SceneArchive(Scene& scene, const AssetPackConfig& assetConfig)
        : _loader{ scene.getRegistry() }
        , _scene{ scene }
        , _assetConfig{ assetConfig }
        , _count{ 0 }
        , _type{ 0 }
    {
    }

    AssetPack& SceneArchive::getAssetPack()
    {
		return _assetPack.value();
    }

    expected<Entity, std::string> SceneArchive::load(const protobuf::Scene& sceneDef)
    {
		_sceneDef = sceneDef;
        _assetPack.emplace(sceneDef.assets(), _assetConfig);
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

    std::pair<uint32_t, const google::protobuf::Any*> SceneArchive::getComponentData()
    {
        auto& typeComps = _sceneDef->registry().components();
        auto itr = typeComps.find(_type);
        if (itr == typeComps.end())
        {
            _error = "Could not find component type";
            return { 0, nullptr };
        }
        auto& comps = itr->second.components();
        auto itr2 = comps.begin();
        std::advance(itr2, _count);
        auto& key = itr2->first;
        if (itr2 == comps.end())
        {
            _error = "Could not find component";
            return { 0, nullptr };
        }
        return { itr2->first, &itr2->second };
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

    IAssetContext& SceneArchive::getAssets()
    {
		return _assetPack.value();
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

    SceneImporterImpl::SceneImporterImpl(Scene& scene, const AssetPackConfig& assetConfig)
        : _archive{ scene, assetConfig }
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

    IComponentLoadContext& SceneImporterImpl::getComponentLoadContext() noexcept
    {
		return _archive;
    }

    AssetPack& SceneImporterImpl::getAssetPack() noexcept
    {
		return _archive.getAssetPack();
    }

    SceneImporter::SceneImporter(Scene& scene, const AssetPackConfig& assetConfig)
        : _impl{ std::make_unique<SceneImporterImpl>(scene, assetConfig) }
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

    IComponentLoadContext& SceneImporter::getComponentLoadContext() noexcept
    {
        return _impl->getComponentLoadContext();
    }

    AssetPack& SceneImporter::getAssetPack() noexcept
    {
        return _impl->getAssetPack();
    }

    SceneLoaderImpl::SceneLoaderImpl(ISceneDefinitionLoader& defLoader, const AssetPackConfig& assetConfig)
        : _defLoader{ defLoader }
        , _assetConfig{ assetConfig }
    {
    }

    expected<Entity, std::string> SceneLoaderImpl::operator()(Scene& scene, std::filesystem::path path)
    {
        auto defResult = _defLoader(path);
        if (!defResult)
        {
            return unexpected<std::string>{ defResult.error() };
        }
        SceneImporter importer{ scene, _assetConfig };
        return importer(*defResult.value());
    }

    SceneLoader::SceneLoader(ISceneDefinitionLoader& defLoader, const AssetPackConfig& assetConfig)
		: _impl{ std::make_unique<SceneLoaderImpl>(defLoader, assetConfig) }
	{
	}

    SceneLoader::~SceneLoader()
    {
        // empty on purpose
    }

    SceneLoader::Result SceneLoader::operator()(Scene& scene, std::filesystem::path path)
    {
        return (*_impl)(scene, path);
    }
}