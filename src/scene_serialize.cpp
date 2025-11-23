#include <darmok/scene_serialize.hpp>
#include <darmok/scene.hpp>
#include "detail/scene_serialize.hpp"

#include <darmok/camera.hpp>
#include <darmok/transform.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/light.hpp>
#include <darmok/string.hpp>
#include <darmok/program_core.hpp>
#include <darmok/mesh_core.hpp>
#include <darmok/shape.hpp>

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

    const ConstSceneDefinitionWrapper::Definition& ConstSceneDefinitionWrapper::getDefinition() const noexcept
    {
        return _def.value();
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

    EntityId ConstSceneDefinitionWrapper::getRootEntity() const noexcept
    {
		auto entities = getRootEntities();
        if (entities.empty())
        {
            return nullEntityId;
        }
        return entities.back();
    }

    std::vector<EntityId> ConstSceneDefinitionWrapper::getRootEntities() const noexcept
    {
        std::vector<EntityId> entities;
        for (auto& [entity, trans] : getTypeComponents<Transform>())
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
            if (!getComponent<Transform>(entity))
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
        for (auto& [childEntity, trans] : getTypeComponents<Transform>())
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

    EntityId ConstSceneDefinitionWrapper::getEntity(const Any& anyComp) const noexcept
    {
        auto typeId = protobuf::getTypeId(anyComp);
        auto typeComps = getTypeComponents(typeId);
        if (!typeComps)
        {
            return nullEntityId;
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
		return nullEntityId;
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
            if (typeId != 0 && protobuf::getTypeId(any) != typeId)
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

    SceneDefinitionWrapper::Definition& SceneDefinitionWrapper::getDefinition() noexcept
    {
        return _def.value();
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

	Scene SceneLoaderImpl::_emptyScene{};
    const SceneLoaderImpl::SceneDefinition SceneLoaderImpl::_emptySceneDef;

    SceneLoaderImpl::SceneLoaderImpl() noexcept
        : _scene{ _emptyScene }
        , _sceneDef{ _emptySceneDef }
        , _loader{ _scene->getRegistry() }
        , _count{ 0 }
        , _typeId{ 0 }
        , _parentEntity{ entt::null }
    {
    }

    void SceneLoaderImpl::createAssetPack() const noexcept
    {
        _assetPack = std::make_unique<AssetPack>(_sceneDef->assets(), _assetConfig);
    }

    IAssetContext& SceneLoaderImpl::getAssets() noexcept
    {
        return getAssetPack();
    }

    AssetPack& SceneLoaderImpl::getAssetPack() noexcept
    {
        if (!_assetPack)
        {
            createAssetPack();
        }
		return *_assetPack;
    }

    const AssetPack& SceneLoaderImpl::getAssetPack() const noexcept
    {
        if (!_assetPack)
        {
            createAssetPack();
        }
        return *_assetPack;
    }

    void SceneLoaderImpl::setParent(Entity entity) noexcept
    {
        _parentEntity = entity;
    }

    void SceneLoaderImpl::setAssetPackConfig(AssetPackConfig assetConfig) noexcept
    {
		_assetConfig = std::move(assetConfig);
    }

    void SceneLoaderImpl::addComponentListener(std::function<void(const Any& compAny, Entity entity)>&& func) noexcept
    {
        _compListeners.push_back(std::move(func));
    }

    void SceneLoaderImpl::clearComponentListeners() noexcept
    {
		_compListeners.clear();
    }

    void SceneLoaderImpl::callComponentListeners(const Any& compAny, Entity entity) noexcept
    {
        for (auto& listener : _compListeners)
        {
            listener(compAny, entity);
        }
    }

    const Scene& SceneLoaderImpl::getScene() const noexcept
    {
        return *_scene;
    }

    Scene& SceneLoaderImpl::getScene() noexcept
    {
        return *_scene;
    }

    Entity SceneLoaderImpl::getEntity(EntityId entityId) const noexcept
    {
        if (entityId == nullEntityId)
        {
            return _parentEntity;
        }
        return _loader.map(Entity{ entityId });
    }

    SceneLoaderImpl::EntityResult SceneLoaderImpl::load(const Scene::Definition& sceneDef, Scene& scene) noexcept
    {
        if (_scene.ptr() != &scene || _sceneDef.ptr() != &sceneDef)
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

        std::vector<std::string> errors;
        for (auto& func : _postLoadFuncs)
        {
            auto result = func();
            if (!result)
            {
                errors.push_back(std::move(result).error());
            }
        }
        _postLoadFuncs.clear();
        if (!errors.empty())
        {
            return unexpected{ StringUtils::joinErrors(errors) };
        }

        auto roots = ConstSceneDefinitionWrapper{ sceneDef }.getRootEntities();
        if (roots.empty())
        {
            return Entity{ entt::null };
        }

        return getEntity(roots.back());
    }

    void SceneLoaderImpl::operator()(std::underlying_type_t<Entity>& count) noexcept
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

    void SceneLoaderImpl::operator()(Entity& entity) noexcept
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

    void SceneLoaderImpl::addError(std::string_view error) noexcept
    {
		_errors.emplace_back(error);
    }

    SceneLoaderImpl::ComponentData SceneLoaderImpl::getComponentData() noexcept
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

    void SceneLoaderImpl::addLoad(LoadFunction&& func)
    {
        _loadFuncs.push_back(std::move(func));
    }

    void SceneLoaderImpl::addPostLoad(LoadFunction&& func)
    {
        _postLoadFuncs.push_back(std::move(func));
        ++_count;
    }

    SceneLoaderImpl::Result SceneLoaderImpl::beforeLoadComponent(IdType typeId) noexcept
    {
        _errors.clear();
        _typeId = typeId;
        return {};
    }

    SceneLoaderImpl::Result SceneLoaderImpl::afterLoadComponent(IdType typeId) noexcept
    {
        if(!_errors.empty())
        {
            auto msg = StringUtils::joinErrors(_errors);
            _errors.clear();
            return unexpected{ std::move(msg) };
		}
        return {};
    }

    entt::continuous_loader& SceneLoaderImpl::getLoader() noexcept
    {
        return _loader;
    }

    SceneArchive::SceneArchive(SceneLoaderImpl& impl) noexcept
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

    void SceneArchive::callComponentListeners(const Any& compAny, Entity entity) noexcept
    {
        _impl.callComponentListeners(compAny, entity);
    }

    SceneLoader::SceneLoader() noexcept
        : _impl{ std::make_unique<SceneLoaderImpl>() }
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
        // registerComponent<BoundingBox>();
	}

    SceneLoader::~SceneLoader() = default;

    SceneLoader::EntityResult SceneLoader::operator()(const SceneDefinition& sceneDef, Scene& scene) noexcept
    {
		return _impl->load(sceneDef, scene);
    }

    SceneArchive& SceneLoader::getArchive() noexcept
    {
        return _archive;
    }

    IComponentLoadContext& SceneLoader::getComponentLoadContext() noexcept
    {
        return *_impl;
    }

    const IComponentLoadContext& SceneLoader::getComponentLoadContext() const noexcept
    {
        return *_impl;
    }
    
    AssetPack& SceneLoader::getAssetPack() noexcept
    {
		return _impl->getAssetPack();
    }

    const AssetPack& SceneLoader::getAssetPack() const noexcept
    {
        return _impl->getAssetPack();
    }

    SceneLoader& SceneLoader::setParent(Entity entity) noexcept
    {
        _impl->setParent(entity);
        return *this;
    }

    SceneLoader& SceneLoader::addComponentListener(std::function<void(const Any& compDef, Entity entity)>&& func) noexcept
    {
        _impl->addComponentListener(std::move(func));
        return *this;
    }

    SceneLoader& SceneLoader::clearComponentListeners() noexcept
    {
        _impl->clearComponentListeners();
        return *this;
    }

    SceneLoader& SceneLoader::setAssetPackConfig(AssetPackConfig assetConfig) noexcept
    {
		_impl->setAssetPackConfig(std::move(assetConfig));
        return *this;
    }

    void SceneLoader::addLoad(LoadFunction&& func)
    {
		return _impl->addLoad(std::move(func));
    }

    SceneLoader::Result SceneLoader::beforeLoadComponent(IdType typeId) noexcept
    {
		return _impl->beforeLoadComponent(typeId);
    }

    SceneLoader::Result SceneLoader::afterLoadComponent(IdType typeId) noexcept
    {
		return _impl->afterLoadComponent(typeId);
    }

    entt::continuous_loader& SceneLoader::getLoader() noexcept
    {
		return _impl->getLoader();
    }

    
    SceneDefinitionCompiler::SceneDefinitionCompiler(const Config& config, OptionalRef<IProgramSourceLoader> progLoader) noexcept
        : _config{ config }
        , _progLoader{ progLoader }
    {
    }

    expected<void, std::string> SceneDefinitionCompiler::operator()(Definition& def)
    {
        SceneDefinitionWrapper scene{ def };
        for (auto& [path, progSrc] : scene.getAssets<Program::Source>())
        {
            ProgramCompiler progCompiler{ _config.progCompiler };
            auto result = progCompiler(progSrc);
            if (!result)
            {
                return unexpected{ fmt::format("failed to compile program {}: {}", path.string(), result.error())};
            }
            scene.setAsset(path, result.value());
        }
        for (auto& [path, meshSrc] : scene.getAssets<Mesh::Source>())
        {
            auto& progRef = meshSrc.program();

            auto progResult = Program::loadRefVarying(progRef, _progLoader);
            if (!progResult)
            {
                return unexpected{ fmt::format("failed to load varying for mesh {}: {}", path.string(), progResult.error()) };
            }
            auto varying = progResult.value();
            auto layout = ConstVertexLayoutWrapper{ varying.vertex() }.getBgfx();
            MeshConfig config{ .index32 = meshSrc.index32() };
            auto def = MeshData{ meshSrc }.createDefinition(layout, config);
            scene.setAsset(path, def);
        }

        return {};
    }
}