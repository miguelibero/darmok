#pragma once

#include <darmok/export.h>
#include <darmok/expected.hpp>
#include <darmok/loader.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/convert.hpp>
#include <darmok/scene.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/program_core.hpp>
#include <darmok/protobuf/scene.pb.h>

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <fmt/format.h>

namespace darmok
{
    class DARMOK_EXPORT ConstSceneDefinitionWrapper
    {
    public:
        using Definition = protobuf::Scene;
        using Message = google::protobuf::Message;
        using Any = google::protobuf::Any;
        using RegistryComponents = protobuf::RegistryComponents;
        using Transform = protobuf::Transform;

        ConstSceneDefinitionWrapper(const Definition& def) noexcept;
        const std::string& getName() const noexcept;
		const Definition& getDefinition() const noexcept;
        OptionalRef<const Any> getSceneComponent(IdType typeId) const noexcept;
		bool hasSceneComponent(IdType typeId) const noexcept;

		EntityId getRootEntity() const noexcept;
        std::vector<EntityId> getRootEntities() const noexcept;
        std::vector<EntityId> getEntities() const noexcept;
        std::vector<EntityId> getChildren(EntityId entity) const noexcept;
        OptionalRef<const RegistryComponents> getTypeComponents(IdType typeId) const noexcept;
        std::vector<std::reference_wrapper<const Any>> getComponents(EntityId entity) const noexcept;
        OptionalRef<const Any> getComponent(EntityId entity, IdType typeId) const noexcept;
		EntityId getEntity(const Any& anyComp) const noexcept;
		bool hasComponent(EntityId entity, IdType typeId) const noexcept;

        std::optional<std::filesystem::path> getAssetPath(const Any& anyAsset) const noexcept;
        std::vector<std::filesystem::path> getAssetPaths(IdType typeId) const noexcept;
        std::unordered_map<std::filesystem::path, OptionalRef<const Any>> getAssets(IdType typeId) const noexcept;
        std::unordered_map<std::filesystem::path, OptionalRef<const Any>> getAssets(const std::filesystem::path& parentPath) const noexcept;
        OptionalRef<const Any> getAsset(const std::filesystem::path& path) const noexcept;
        bool hasAsset(const std::filesystem::path& path) const noexcept;

        template<typename T>
        std::optional<T> getSceneComponent() const noexcept
        {
            if (auto any = getSceneComponent(protobuf::getTypeId<T>()))
            {
                T comp;
                if (any->UnpackTo(&comp))
                {
                    return comp;
                }
            }
            return std::nullopt;
        }

        template<typename T>
        bool hasSceneComponent() const noexcept
        {
            return hasSceneComponent(protobuf::getTypeId<T>());
        }

        template<typename T>
        std::unordered_map<EntityId, T> getTypeComponents() const noexcept
        {
            std::unordered_map<EntityId, T> comps;
            if (auto typeComps = getTypeComponents(protobuf::getTypeId<T>()))
            {
                for(auto& [entityId, any] : typeComps->components())
                {
                    T comp;
                    if (any.UnpackTo(&comp))
                    {
                        comps[entityId] = std::move(comp);
                    }
				}
            }
            return comps;
        }

        template<typename T>
        std::optional<T> getComponent(EntityId entity) const noexcept
        {
            if (auto any = getComponent(entity, protobuf::getTypeId<T>()))
            {
                T comp;
                if (any->UnpackTo(&comp))
                {
                    return comp;
                }
            }
            return std::nullopt;
        }

        template<typename T>
        bool hasComponent(EntityId entity) const noexcept
        {
			return hasComponent(entity, protobuf::getTypeId<T>());
        }

        template<typename T>
        std::vector<std::filesystem::path> getAssetPaths() const noexcept
        {
            return getAssetPaths(protobuf::getTypeId<T>());
        }

        template<typename T>
        std::unordered_map<std::filesystem::path, T> getAssets() const noexcept
        {
            std::unordered_map<std::filesystem::path, T> assets;
            for (auto& [path, any] : getAssets(protobuf::getTypeId<T>()))
            {
                T asset;
                if (any->UnpackTo(&asset))
                {
                    assets[path] = std::move(asset);
                }
            }
            return assets;
        }

        template<typename T>
        std::optional<T> getAsset(const std::filesystem::path& path) const noexcept
        {
            if (auto any = getAsset(path))
            {
                T asset;
                if (any->UnpackTo(&asset))
                {
                    return asset;
                }
            }
            return std::nullopt;
        }

        template<typename C>
        bool forEachEntity(const C& callback)
        {
            for (auto& entity : getEntities())
            {
                if (callback(entity))
                {
                    return true;
                }
            }
            return false;
        }

        template<typename C>
        bool forEachParent(EntityId entity, const C& callback)
        {
            if (entity == entt::null)
            {
                return false;
            }
            auto optTrans = getComponent<Transform>(entity);
            if (!optTrans)
            {
                return false;
            }
            auto& trans = optTrans.value();
            if (callback(entity, trans))
            {
                return true;
            }
            auto parent = trans.parent();
            if (parent == nullEntityId)
            {
                return false;
            }
            return forEachParent(parent, callback);
        }

        template<typename C>
        bool forEachChild(const C& callback)
        {
            for (auto root : getRootEntities())
            {
                if (forEachChild(root, callback))
                {
                    return true;
                }
            }
            return false;
        }

        template<typename C>
        bool forEachChild(EntityId entity, const C& callback)
        {
            if (entity == entt::null)
            {
                return false;
            }
            auto optTrans = getComponent<Transform>(entity);
            if (!optTrans)
            {
                return false;
            }
            auto& trans = optTrans.value();
            if (callback(entity, trans))
            {
                return true;
            }
            for (auto& entity : getChildren(entity))
            {
                if (forEachChild(entity, callback))
                {
                    return true;
                }
            }
            return false;
        }

    protected:
        static std::optional<std::filesystem::path> getChildPath(const std::filesystem::path& path, const std::filesystem::path& parentPath) noexcept;
    private:
        OptionalRef<const Definition> _def;
    };

	class DARMOK_EXPORT SceneDefinitionWrapper final : public ConstSceneDefinitionWrapper
    {
    public:
        using Definition = protobuf::Scene;
        using Message = google::protobuf::Message;
        using Any = google::protobuf::Any;
        using RegistryComponents = protobuf::RegistryComponents;

        SceneDefinitionWrapper(Definition& def) noexcept;
        void setName(std::string_view name) noexcept;
        Definition& getDefinition() noexcept;
        bool setSceneComponent(const Message& comp) noexcept;
		bool removeSceneComponent(IdType typeId) noexcept;

        EntityId createEntity() noexcept;
        bool setAsset(const std::filesystem::path& path, const Message& asset) noexcept;
        std::filesystem::path addAsset(const std::filesystem::path& pathPrefix, const Message& asset) noexcept;
        bool setComponent(EntityId entity, const Message& comp) noexcept;
        std::vector<std::reference_wrapper<Any>> getComponents(EntityId entity) noexcept;
        OptionalRef<RegistryComponents> getTypeComponents(IdType typeId) noexcept;
        OptionalRef<Any> getComponent(EntityId entity, IdType typeId) noexcept;
        bool destroyEntity(EntityId entity) noexcept;
        bool removeComponent(EntityId entity, IdType typeId) noexcept;
        bool removeComponent(const Any& anyComp) noexcept;

        std::unordered_map<std::filesystem::path, OptionalRef<Any>> getAssets(IdType typeId = 0) noexcept;
        std::unordered_map<std::filesystem::path, OptionalRef<Any>> getAssets(const std::filesystem::path& parentPath) noexcept;
        OptionalRef<Any> getAsset(const std::filesystem::path& path);
        bool removeAsset(const std::filesystem::path& path) noexcept;
        bool removeAsset(const Any& anyAsset) noexcept;

        template<typename T>
        bool removeSceneComponent() noexcept
        {
            return removeSceneComponent(protobuf::getTypeId<T>());
        }

        template<typename T>
        std::optional<T> getComponent(EntityId entity) const noexcept
        {
            if (auto any = ConstSceneDefinitionWrapper::getComponent(entity, protobuf::getTypeId<T>()))
            {
                T asset;
                if (any->UnpackTo(&asset))
                {
                    return asset;
                }
            }
            return std::nullopt;
        }

        template<typename T>
        std::optional<T> getAsset(const std::filesystem::path& path) const noexcept
        {
            return ConstSceneDefinitionWrapper::getAsset<T>(path);
        }

        template<typename T>
        std::unordered_map<std::filesystem::path, T> getAssets() const noexcept
        {
            return ConstSceneDefinitionWrapper::getAssets<T>();
        }

        template<typename T>
        bool removeAsset(const std::filesystem::path& path) noexcept
        {
			return removeAsset(protobuf::getTypeId<T>(), path);
        }

        template<typename T>
        bool removeComponent(EntityId entity) noexcept
        {
            return removeComponent(entity, protobuf::getTypeId<T>());
        }

    private:
		OptionalRef<Definition> _def;
    };

    class DARMOK_EXPORT BX_NO_VTABLE ISceneDefinitionLoader : public ILoader<protobuf::Scene>{};

    class App;
    class Scene;
    class IAssetContext;

    class DARMOK_EXPORT BX_NO_VTABLE IComponentLoadContext
    {
    public:
		virtual ~IComponentLoadContext() = default;
        virtual IAssetContext& getAssets() noexcept = 0;
        virtual const Scene& getScene() const noexcept = 0;
        virtual Scene& getScene() noexcept = 0;
        virtual const App& getApp() const noexcept = 0;
        virtual App& getApp() noexcept = 0;
        virtual Entity getEntity(EntityId entityId) const noexcept = 0;
    };

	using DataSceneDefinitionLoader = DataProtobufLoader<ISceneDefinitionLoader>;

    class Scene;
    class AssetPack;
    class AssetPackConfig;

    struct ComponentRef final
    {
        Entity entity;
        IdType type;
    };

    template<typename Arg>
    class DARMOK_EXPORT BX_NO_VTABLE IBasicSceneLoader
    {
    public:
        using Error = std::string;
        using Definition = protobuf::Scene;
        using Result = expected<Entity, Error>;
        using Argument = Arg;
        virtual ~IBasicSceneLoader() = default;
        virtual Result operator()(Scene& scene, Argument arg) = 0;
    };

    struct SceneConverterComponentData
    {
        EntityId entityId;
        OptionalRef<const google::protobuf::Any> any;
    };

    class SceneLoaderImpl;

    template <typename T, typename Def = T::Definition>
    concept HasBasicSceneComponentLoad = HasDefinitionLoad<T, Def>;

    template <typename T, typename Def = T::Definition>
    concept HasContextSceneComponentLoad = requires(T& c, const Def& def, IComponentLoadContext& ctx) {
        { c.load(def, ctx) } -> std::same_as<expected<void, std::string>>;
    };

    class SceneArchive final
    {
    public:
        using Result = expected<void, std::string>;
        using LoadFunction = std::function<Result()>;
        using ComponentData = SceneConverterComponentData;
        using Message = protobuf::Message;
        using Any = google::protobuf::Any;

        SceneArchive(SceneLoaderImpl& impl) noexcept;

        void operator()(std::underlying_type_t<Entity>& count) noexcept;
        void operator()(Entity& entity) noexcept;

        template<typename T>
        void operator()(T& obj) noexcept
        {
            auto data = getComponentData();
            if (!data.any)
            {
                return;
            }
            // we need to delay the load() calls to guarantee that
            // all the components are present before
            addPostLoad([this, data = std::move(data)]()
            {
                return doLoad<T>(std::move(data));
            });
        }

        template<typename T, typename Def = T::Definition>
        static expected<void, std::string> loadComponent(T& comp, const Def& def, OptionalRef<IComponentLoadContext> ctxt = nullptr) noexcept
        {
            expected<void, std::string> result;
            if constexpr (HasContextSceneComponentLoad<T, Def>)
            {
                if (!ctxt)
                {
                    return unexpected<std::string>{"missing component load context"};
                }
                return comp.load(def, *ctxt);
            }
            else if constexpr (HasBasicSceneComponentLoad<T, Def>)
            {
                return comp.load(def);
            }
            else if constexpr (std::is_assignable_v<T&, Def>)
            {
                comp = def;
                return {};
            }
            else if constexpr (IsConvertible<T, Def> && std::is_assignable_v<T&, T>)
            {
                comp = convert<T>(def);
                return {};
            }
            else
            {
                return unexpected<std::string>{ "could not load" };
            }
        }

        template<typename T, typename Def = T::Definition>
        expected<std::reference_wrapper<T>, std::string> loadSceneComponent(Def def) noexcept
        {
            auto compResult = getScene().getOrAddSceneComponent<T>();
            if (!compResult)
            {
                return compResult;
            }
            auto loadResult = loadComponent<T, Def>(compResult.value().get(), def, getComponentLoadContext());
            if (!loadResult)
            {
                return unexpected{ std::move(loadResult).error() };
            }
            return compResult;

            /*
            // we need to delay the load() calls to guarantee that
            // all the components are present before
            addPostLoad([this, &comp, def = std::move(def)]()
            {
                loadComponent(comp, def, getComponentLoadContext());
            });
            */
        }

    private:
        SceneLoaderImpl& _impl;

		template<typename T>
        expected<void, std::string> doLoad(ComponentData data) noexcept
        {
            if (!data.any)
            {
                return unexpected<std::string>{"Empty any when loading component"};
            }
            auto entity = getEntity(data.entityId);
            auto comp = getScene().getComponent<T>(entity);
            if(!comp)
            {
                return unexpected<std::string>{"Missing scene component"};
			}

			using Def = T::Definition;
            Def def;
            if (!data.any->UnpackTo(&def))
            {
                return unexpected<std::string>{"Could not unpack component"};
            }

            auto result = SceneArchive::loadComponent(*comp, def, getComponentLoadContext());
            if (!result)
            {
                return unexpected{ std::move(result).error() };
            }
           
            return callComponentListeners(*data.any, entity);
        }

        Entity getEntity(EntityId entityId) const noexcept;
        Scene& getScene() noexcept;
        void addError(std::string_view error) noexcept;
        ComponentData getComponentData() noexcept;
        IComponentLoadContext& getComponentLoadContext() noexcept;
        void addPostLoad(LoadFunction&& func);
        Result callComponentListeners(const Any& compAny, Entity entity) noexcept;
    };

    class SceneLoader final
    {
    public:
        using EntityResult = expected<Entity, std::string>;
        using Result = expected<void, std::string>;
        using LoadFunction = std::function<Result()>;
		using SceneDefinition = protobuf::Scene;
        using ComponentData = SceneConverterComponentData;
        using Message = protobuf::Message;
        using Any = google::protobuf::Any;
        using ComponentListener = std::function<expected<void, std::string>(const Any& compAny, Entity entity)>;

        SceneLoader(App& app) noexcept;
        ~SceneLoader() noexcept;
        SceneLoader(const SceneLoader& other) = delete;
        SceneLoader& operator=(const SceneLoader& other) = delete;

        template<typename T, typename Def = T::Definition>
        void registerComponent()
        {
            auto func = [this]() -> expected<void, std::string>
            {
                return loadComponent<T, Def>();
            };
            addLoad(std::move(func));
        }

        template<typename T, typename Def = T::Definition>
            requires std::is_base_of_v<ISceneComponent, T>
        void registerSceneComponent()
        {
            auto func = [this]() -> expected<void, std::string>
            {
                auto result = loadSceneComponent<T, Def>();
                if (!result)
                {
                    return unexpected{ std::move(result).error() };
                }
                return {};
            };
            addLoad(std::move(func));
        }

        template<typename T>
            requires std::is_base_of_v<ICameraComponent, T>
        void registerCameraComponent()
        {
        }

        EntityResult operator()(const SceneDefinition& sceneDef, Scene& scene) noexcept;

        IComponentLoadContext& getComponentLoadContext() noexcept;
        const IComponentLoadContext& getComponentLoadContext() const noexcept;
        AssetPack& getAssetPack() noexcept;
        const AssetPack& getAssetPack() const noexcept;

        SceneLoader& setParent(Entity entity) noexcept;
        SceneLoader& setAssetPackConfig(AssetPackConfig assetConfig) noexcept;

        template<typename T>
        SceneLoader& addComponentListener(std::function<void(const typename T::Definition&, Entity)>&& func) noexcept
        {
            return addComponentListener([func = std::move(func)](const Any& compAny, Entity entity)
            {
				typename T::Definition compDef;
                if (compAny.UnpackTo(&compDef))
                {
                    func(static_cast<const T::Definition&>(compDef), entity);
                }
            });
        }

        SceneLoader& addComponentListener(ComponentListener func) noexcept;
		SceneLoader& clearComponentListeners() noexcept;

    private:
		std::unique_ptr<SceneLoaderImpl> _impl;
        SceneArchive _archive;

        template<typename T, typename Def = typename T::Definition>
        Result loadComponent() noexcept
        {
            auto typeId = protobuf::getTypeId<Def>();
            auto result = beforeLoadComponent(typeId);
            if (!result)
            {
                return result;
            }
            getLoader().get<T>(getArchive());
            return afterLoadComponent(typeId);
        }

        template<typename T, typename Def = typename T::Definition>
        expected<OptionalRef<T>, std::string> loadSceneComponent() noexcept
        {
            ConstSceneDefinitionWrapper sceneDef{ getSceneDefinition() };
            if(auto def = sceneDef.getSceneComponent<Def>())
            {
                auto result = getArchive().loadSceneComponent<T>(std::move(*def));
                if (!result)
                {
                    return unexpected{ std::move(result).error() };
                }
                return result.value().get();
            }
            return OptionalRef<T>{};
        }

        void addLoad(LoadFunction&& func);
        Result beforeLoadComponent(IdType typeId) noexcept;
        Result afterLoadComponent(IdType typeId) noexcept;
		entt::continuous_loader& getLoader() noexcept;
        SceneArchive& getArchive() noexcept;
        const SceneDefinition& getSceneDefinition() const noexcept;
    };

    struct DARMOK_EXPORT SceneDefinitionCompilerConfig final
    {
        ProgramCompilerConfig progCompiler;
        OptionalRef<bx::AllocatorI> alloc;
    };

    class DARMOK_EXPORT SceneDefinitionCompiler final
    {
    public:
        using Config = SceneDefinitionCompilerConfig;
        using Definition = protobuf::Scene;

        SceneDefinitionCompiler(const Config& config = {}, OptionalRef<IProgramSourceLoader> progLoader = nullptr) noexcept;
        expected<void, std::string> operator()(Definition& def);
    private:
        Config _config;
        bx::DefaultAllocator _defAlloc;
        OptionalRef<IProgramSourceLoader> _progLoader;
    };
}

namespace fmt
{
    template<>
    struct formatter<darmok::Entity> : public formatter<entt::entt_traits<darmok::Entity>::entity_type>
    {
        template <typename FormatContext>
        auto format(const darmok::Entity& v, FormatContext& ctx) const
        {
            return formatter<entt::entt_traits<darmok::Entity>::entity_type>::format(entt::to_integral(v), ctx);
        }
    };
}