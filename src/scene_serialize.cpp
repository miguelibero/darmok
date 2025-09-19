#include <darmok/scene_serialize.hpp>
#include <darmok/scene.hpp>
#include "detail/scene_serialize.hpp"

#include <darmok/camera.hpp>
#include <darmok/transform.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/light.hpp>
#include <darmok/string.hpp>

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

    std::vector<EntityId> ConstSceneDefinitionWrapper::getEntities() const noexcept
    {
        std::vector<EntityId> entities;
        for (auto& [type, typeComps] : _def->registry().components())
        {
            for (auto& [entityId, comp] : typeComps.components())
            {
                if (std::find(entities.begin(), entities.end(), entityId) == entities.end())
                {
                    entities.push_back(entityId);
                }
            }
        }
        std::sort(entities.begin(), entities.end());
        return entities;
    }

    std::vector<EntityId> ConstSceneDefinitionWrapper::getRootEntities() const noexcept
    {
        std::vector<EntityId> entities;
        for (auto& [entity, trans] : getTypeComponents<Transform::Definition>())
        {
            if (!trans.has_parent() || trans.parent() == 0)
            {
                entities.push_back(entity);
            }
        }
        for (auto& entity : getEntities())
        {
            if (std::find(entities.begin(), entities.end(), entity) != entities.end())
            {
                continue;
            }
            if (!getComponent<Transform::Definition>(entity))
            {
                entities.push_back(entity);
            }
        }
        std::sort(entities.begin(), entities.end());
        return entities;
    }

    std::vector<EntityId> ConstSceneDefinitionWrapper::getChildren(EntityId entity) const noexcept
    {
        std::vector<EntityId> entities;
        for (auto& [childEntity, trans] : getTypeComponents<Transform::Definition>())
        {
            if (trans.parent() == entity)
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

    OptionalRef<const ConstSceneDefinitionWrapper::Any> ConstSceneDefinitionWrapper::getComponent(EntityId entity, IdType typeId) const noexcept
    {
        auto typeComps = getTypeComponents(typeId);
        if (!typeComps)
        {
            return std::nullopt;
        }
        auto& components = typeComps->components();
        auto itr = components.find(entity);
        if (itr == components.end())
        {
            return std::nullopt;
        }
        return itr->second;
    }

    std::optional<EntityId> ConstSceneDefinitionWrapper::getEntity(const Any& anyComp) const noexcept
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
            return itr->first;
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

    EntityId SceneDefinitionWrapper::createEntity() noexcept
    {
        auto& reg = *_def->mutable_registry();
        auto v = reg.entities() + 1;
        reg.set_entities(v);
        return v;
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

    bool SceneDefinitionWrapper::setComponent(EntityId entity, const Message& comp) noexcept
    {
        auto typeId = protobuf::getTypeId(comp);
        auto& components = _def->mutable_registry()->mutable_components()->try_emplace(typeId).first->second;
        auto result = components.mutable_components()->try_emplace(entity);
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

    std::vector<std::reference_wrapper<SceneDefinitionWrapper::Any>> SceneDefinitionWrapper::getComponents(EntityId entity) noexcept
    {
        std::vector<std::reference_wrapper<Any>> comps;
        for(auto& [typeId, typeComps] : *_def->mutable_registry()->mutable_components())
        {
            auto& components = *typeComps.mutable_components();
            auto itr = components.find(entity);
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

    OptionalRef<SceneDefinitionWrapper::Any> SceneDefinitionWrapper::getComponent(EntityId entity, IdType typeId) noexcept
    {
		auto typeComps = getTypeComponents(typeId);
        if(!typeComps)
        {
            return std::nullopt;
        }
        auto& components = *typeComps->mutable_components();
        auto itr = components.find(entity);
        if (itr == components.end())
        {
            return std::nullopt;
		}
		return itr->second;
    }

    bool SceneDefinitionWrapper::destroyEntity(EntityId entity) noexcept
    {
        bool found = false;
        for (auto& [typeId, typeComps] : *_def->mutable_registry()->mutable_components())
        {
            auto& components = *typeComps.mutable_components();
            auto itr = components.find(entity);
            if (itr != components.end())
            {
                components.erase(itr);
                found = true;
            }
        }
        return found;
    }

    bool SceneDefinitionWrapper::removeComponent(EntityId entity, IdType typeId) noexcept
    {
        auto typeComps = getTypeComponents(typeId);
        if (!typeComps)
        {
            return false;
        }
        return typeComps->mutable_components()->erase(entity) > 0;
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

	Scene SceneConverterImpl::_emptyScene{};
    const SceneConverterImpl::SceneDefinition SceneConverterImpl::_emptySceneDef;

    SceneConverterImpl::SceneConverterImpl() noexcept
        : _scene{ _emptyScene }
        , _sceneDef{ _emptySceneDef }
        , _loader{ _scene->getRegistry() }
        , _count{ 0 }
        , _typeId{ 0 }
    {
    }

    void SceneConverterImpl::createAssetPack() const noexcept
    {
        _assetPack = std::make_unique<AssetPack>(_sceneDef->assets(), _assetConfig);
    }

    IAssetContext& SceneConverterImpl::getAssets() noexcept
    {
        return getAssetPack();
    }

    AssetPack& SceneConverterImpl::getAssetPack() noexcept
    {
        if (!_assetPack)
        {
            createAssetPack();
        }
		return *_assetPack;
    }

    const AssetPack& SceneConverterImpl::getAssetPack() const noexcept
    {
        if (!_assetPack)
        {
            createAssetPack();
        }
        return *_assetPack;
    }

    void SceneConverterImpl::setAssetPackConfig(AssetPackConfig assetConfig) noexcept
    {
		_assetConfig = std::move(assetConfig);
    }

    const Scene& SceneConverterImpl::getScene() const noexcept
    {
        return *_scene;
    }

    Scene& SceneConverterImpl::getScene() noexcept
    {
        return *_scene;
    }

    Entity SceneConverterImpl::getEntity(EntityId entityId) const noexcept
    {
        return _loader.map(Entity{ entityId });
    }

    expected<Entity, std::string> SceneConverterImpl::load(const Scene::Definition& sceneDef, Scene& scene) noexcept
    {
        if (_scene.ptr() != &scene)
        {
			_loader = entt::continuous_loader{ scene.getRegistry() };
        }
        _sceneDef = sceneDef;
		_scene = scene;
        _typeId = 0;
        _count = 0;
        createAssetPack();

        _loader.get<entt::entity>(*this);
        
        for (auto& func : _loadFuncs)
        {
            auto result = func();
            if (!result)
            {
                return unexpected{ result.error() };
            }
        }

        _loader.orphans();

        Result result;
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
        if (roots.empty())
        {
            return unexpected{ "No root entities found in the scene definition." };
        }
        return getEntity(roots.back());
    }

    void SceneConverterImpl::operator()(std::underlying_type_t<Entity>& count) noexcept
    {
        if (!_sceneDef)
        {
            addError("Scene definition is not loaded.");
            count = 0;
            return;
        }
        auto& reg = _sceneDef->registry();
        if (_typeId == 0)
        {
            count = reg.entities();
        }
        else
        {
            count = 0;
            auto& typeComps = reg.components();
            auto itr = typeComps.find(_typeId);
            if (itr != typeComps.end())
            {
                count = itr->second.components_size();
            }
        }
        _count = 0;
    }

    void SceneConverterImpl::operator()(Entity& entity) noexcept
    {
        if (_typeId == 0)
        {
            entity = static_cast<Entity>(++_count);
        }
        else
        {
            auto data = getComponentData();
            entity = Entity{ data.entityId };
        }
    }

    void SceneConverterImpl::addError(std::string_view error) noexcept
    {
		_errors.emplace_back(error);
    }

    SceneConverterImpl::ComponentData SceneConverterImpl::getComponentData() noexcept
    {
        auto& typeComps = _sceneDef->registry().components();
        auto itr = typeComps.find(_typeId);
        if (itr == typeComps.end())
        {
            addError("Could not find component type");
            return { entt::null, nullptr };
        }
        auto& comps = itr->second.components();
        auto itr2 = comps.begin();
        std::advance(itr2, _count);
        auto& key = itr2->first;
        if (itr2 == comps.end())
        {
            addError("Could not find component");
            return { entt::null, nullptr };
        }
        return { itr2->first, &itr2->second };
    }

    void SceneConverterImpl::addLoad(LoadFunction&& func)
    {
        _loadFuncs.push_back(std::move(func));
    }

    void SceneConverterImpl::addPostLoad(LoadFunction&& func)
    {
        _postLoadFuncs.push_back(std::move(func));
        ++_count;
    }

    SceneConverterImpl::Result SceneConverterImpl::beforeLoadComponent(IdType typeId) noexcept
    {
        _errors.clear();
        _typeId = typeId;
        return {};
    }

    SceneConverterImpl::Result SceneConverterImpl::afterLoadComponent(IdType typeId) noexcept
    {
        if(!_errors.empty())
        {
            auto msg = StringUtils::join("\n", _errors);
            _errors.clear();
            return unexpected{ std::move(msg) };
		}
        return {};
    }

    entt::continuous_loader& SceneConverterImpl::getLoader() noexcept
    {
        return _loader;
    }

    SceneArchive::SceneArchive(SceneConverterImpl& impl) noexcept
        : _impl{ impl }
    {
    }

    void SceneArchive::operator()(std::underlying_type_t<Entity>& count) noexcept
    {
        _impl(count);
    }

    void SceneArchive::operator()(Entity& entity) noexcept
    {
        _impl(entity);
    }

    Entity SceneArchive::getEntity(EntityId entityId) const noexcept
    {
        return _impl.getEntity(entityId);
    }

    Scene& SceneArchive::getScene() noexcept
    {
        return _impl.getScene();
    }

    void SceneArchive::addError(std::string_view error) noexcept
    {
        _impl.addError(error);
    }

    SceneArchive::ComponentData SceneArchive::getComponentData() noexcept
    {
        return _impl.getComponentData();
    }

    IComponentLoadContext& SceneArchive::getComponentLoadContext() noexcept
    {
        return _impl;
    }

    void SceneArchive::addPostLoad(LoadFunction&& func)
    {
        _impl.addPostLoad(std::move(func));
    }

    SceneConverter::SceneConverter() noexcept
        : _impl{ std::make_unique<SceneConverterImpl>() }
        , _archive{ *_impl }
    {
        registerComponent<Transform>();
        registerComponent<Renderable>();
        registerComponent<Camera>();
        registerComponent<Skinnable>();
        registerComponent<PointLight>();
        registerComponent<DirectionalLight>();
        registerComponent<SpotLight>();
        registerComponent<AmbientLight>();
	}

    SceneConverter::~SceneConverter() noexcept = default;

    expected<Entity, std::string> SceneConverter::operator()(const SceneDefinition& sceneDef, Scene& scene) noexcept
    {
		return _impl->load(sceneDef, scene);
    }

    SceneArchive& SceneConverter::getArchive() noexcept
    {
        return _archive;
    }

    IComponentLoadContext& SceneConverter::getComponentLoadContext() noexcept
    {
        return *_impl;
    }
    
    AssetPack& SceneConverter::getAssetPack() noexcept
    {
		return _impl->getAssetPack();
    }

    const AssetPack& SceneConverter::getAssetPack() const noexcept
    {
        return _impl->getAssetPack();
    }

    SceneConverter& SceneConverter::setAssetPackConfig(AssetPackConfig assetConfig) noexcept
    {
		_impl->setAssetPackConfig(std::move(assetConfig));
        return *this;
    }

    void SceneConverter::addLoad(LoadFunction&& func)
    {
		return _impl->addLoad(std::move(func));
    }

    SceneConverter::Result SceneConverter::beforeLoadComponent(IdType typeId) noexcept
    {
		return _impl->beforeLoadComponent(typeId);
    }

    SceneConverter::Result SceneConverter::afterLoadComponent(IdType typeId) noexcept
    {
		return _impl->afterLoadComponent(typeId);
    }

    entt::continuous_loader& SceneConverter::getLoader() noexcept
    {
		return _impl->getLoader();
    }

    SceneLoader::SceneLoader(ISceneDefinitionLoader& defLoader, const AssetPackConfig& assetConfig)
        : _defLoader{ defLoader }
    {
        _converter.setAssetPackConfig(assetConfig);
    }

    expected<Entity, std::string> SceneLoader::operator()(Scene& scene, std::filesystem::path path)
    {
        auto defResult = (*_defLoader)(path);
        if (!defResult)
        {
            return unexpected<std::string>{ defResult.error() };
        }
        return _converter(*defResult.value(), scene);
    }
}