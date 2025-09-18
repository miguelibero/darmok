#pragma once

#include <darmok/export.h>
#include <darmok/expected.hpp>
#include <darmok/loader.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/protobuf/scene.pb.h>

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace darmok
{
    class DARMOK_EXPORT ConstSceneDefinitionWrapper
    {
    public:
        using Definition = protobuf::Scene;
        using Message = google::protobuf::Message;
        using Any = google::protobuf::Any;
        using RegistryComponents = protobuf::RegistryComponents;
        using IdType = entt::id_type;

        ConstSceneDefinitionWrapper(const Definition& def) noexcept;
        const std::string& getName() const noexcept;

        std::vector<Entity> getRootEntities() const noexcept;
        std::vector<Entity> getEntities() const noexcept;
        std::vector<Entity> getChildren(Entity entity) const noexcept;
        OptionalRef<const RegistryComponents> getTypeComponents(IdType typeId) const noexcept;
        std::vector<std::reference_wrapper<const Any>> getComponents(Entity entityId) const noexcept;
        OptionalRef<const Any> getComponent(Entity entity, IdType typeId) const noexcept;
		std::optional<Entity> getEntity(const Any& anyComp) const noexcept;

        std::optional<std::filesystem::path> getAssetPath(const Any& anyAsset) const noexcept;
        std::vector<std::filesystem::path> getAssetPaths(IdType typeId) const noexcept;
        std::unordered_map<std::filesystem::path, OptionalRef<const Any>> getAssets(IdType typeId) const noexcept;
        std::unordered_map<std::filesystem::path, OptionalRef<const Any>> getAssets(const std::filesystem::path& parentPath) const noexcept;
        OptionalRef<const Any> getAsset(const std::filesystem::path& path) const noexcept;

        template<typename T>
        std::unordered_map<Entity, T> getTypeComponents() const noexcept
        {
            std::unordered_map<Entity, T> comps;
            if (auto typeComps = getTypeComponents(protobuf::getTypeId<T>()))
            {
                for(auto& [entityId, any] : typeComps->components())
                {
                    T asset;
                    if (any.UnpackTo(&asset))
                    {
                        comps[Entity{ entityId }] = std::move(asset);
                    }
				}
            }
            return comps;
        }

        template<typename T>
        std::optional<T> getComponent(Entity entity) const noexcept
        {
            if (auto any = getComponent(entity, protobuf::getTypeId<T>()))
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
            if (auto any = getAsset(protobuf::getTypeId<T>(), path))
            {
                T asset;
                if (any->UnpackTo(&asset))
                {
                    return asset;
                }
            }
            return std::nullopt;
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

        Entity createEntity() noexcept;
        bool setAsset(const std::filesystem::path& path, const Message& asset) noexcept;
        std::filesystem::path addAsset(const std::filesystem::path& pathPrefix, const Message& asset) noexcept;
        bool setComponent(Entity entity, const Message& comp) noexcept;
        std::vector<std::reference_wrapper<Any>> getComponents(Entity entity) noexcept;
        OptionalRef<RegistryComponents> getTypeComponents(IdType typeId) noexcept;
        OptionalRef<Any> getComponent(Entity entity, IdType typeId) noexcept;
        bool destroyEntity(Entity entity) noexcept;
        bool removeComponent(Entity entity, IdType typeId) noexcept;
        bool removeComponent(const Any& anyComp) noexcept;

        std::unordered_map<std::filesystem::path, OptionalRef<Any>> getAssets(IdType typeId = 0) noexcept;
        std::unordered_map<std::filesystem::path, OptionalRef<Any>> getAssets(const std::filesystem::path& parentPath) noexcept;
        OptionalRef<Any> getAsset(const std::filesystem::path& path);
        bool removeAsset(const std::filesystem::path& path) noexcept;
        bool removeAsset(const Any& anyAsset) noexcept;

        template<typename T>
        std::optional<T> getComponent(Entity entity) const noexcept
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

        template<typename T>
        bool removeAsset(const std::filesystem::path& path) noexcept
        {
			return removeAsset(protobuf::getTypeId<T>(), path);
        }

        template<typename T>
        bool removeComponent(Entity entity) noexcept
        {
            return removeComponent(entity, protobuf::getTypeId<T>());
        }

    private:
		OptionalRef<Definition> _def;
    };

    using ISceneDefinitionLoader = ILoader<protobuf::Scene>;
    class Scene;
    class IAssetContext;

    class DARMOK_EXPORT BX_NO_VTABLE IComponentLoadContext
    {
    public:
		virtual ~IComponentLoadContext() = default;
        virtual IAssetContext& getAssets() noexcept = 0;
        virtual const Scene& getScene() const noexcept = 0;
        virtual Scene& getScene() noexcept = 0;
        virtual Entity getEntity(uint32_t entityId) const noexcept = 0;
    };

	using DataSceneDefinitionLoader = DataProtobufLoader<ISceneDefinitionLoader>;

    class Scene;
    class AssetPack;
    class AssetPackConfig;

    struct ComponentRef final
    {
        Entity entity;
        uint32_t type;
    };

    template<typename Arg>
    class DARMOK_EXPORT BX_NO_VTABLE IBasicSceneLoader
    {
    public:
        using Error = std::string;
        using Result = expected<Entity, Error>;
        using Argument = Arg;
        virtual ~IBasicSceneLoader() = default;
        virtual Result operator()(Scene& scene, Argument arg) = 0;
    };

    class DARMOK_EXPORT BX_NO_VTABLE ISceneLoader : public IBasicSceneLoader<std::filesystem::path>
    {
    };

    class SceneArchiveImpl;

    class SceneArchive final
    {
    public:
        using Result = expected<void, std::string>;
        using LoadFunction = std::function<Result()>;
		using SceneDefinition = protobuf::Scene;

        struct ComponentData
        {
            uint32_t entityId;
            OptionalRef<const google::protobuf::Any> any;
        };

        SceneArchive() noexcept;
        ~SceneArchive() noexcept;

		SceneArchive(const SceneArchive& other) = delete;
		SceneArchive& operator=(const SceneArchive& other) = delete;

        template<typename T>
        void registerComponent()
        {
            auto func = [this]() -> expected<void, std::string>
            {
                return loadComponent<T>();
            };
            addLoad(std::move(func));
        }

        expected<Entity, std::string> load(const SceneDefinition& sceneDef, Scene& scene);

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
            typename T::Definition def;
            if (!data.any->UnpackTo(&def))
            {
                addError("Could not unpack component");
                return;
            }

            // we need to delay the load() calls to guarantee that
            // all the components are present before
            auto entity = getEntity(data.entityId);
            auto func = [this, entity, def = std::move(def)]() -> expected<void, std::string>
            {
                if (auto comp = getScene().getComponent<T>(entity))
                {
                    return comp->load(def, getComponentLoadContext());
                }
                return {};
            };
			addPostLoad(std::move(func));
        }

        IComponentLoadContext& getComponentLoadContext() noexcept;
        AssetPack& getAssetPack() noexcept;
        const AssetPack& getAssetPack() const noexcept;

        SceneArchive& setAssetPackConfig(AssetPackConfig assetConfig) noexcept;

    private:
		std::unique_ptr<SceneArchiveImpl> _impl;

        template<typename T>
        Result loadComponent()
        {
            using Def = typename T::Definition;
            auto typeId = protobuf::getTypeId<Def>();
            auto result = beforeLoadComponent(typeId);
            if (!result)
            {
                return result;
            }
            getLoader().get<T>(*this);
            return afterLoadComponent(typeId);
        }

		Scene& getScene() noexcept;
        Entity getEntity(uint32_t entityId) const noexcept;
		void addError(std::string_view error) noexcept;
        ComponentData getComponentData() noexcept;
        void addLoad(LoadFunction&& func);
        void addPostLoad(LoadFunction&& func);
        Result beforeLoadComponent(uint32_t typeId) noexcept;
        Result afterLoadComponent(uint32_t typeId) noexcept;
		entt::continuous_loader& getLoader() noexcept;
    };

    class DARMOK_EXPORT SceneLoader final : public ISceneLoader
    {
    public:
        SceneLoader(ISceneDefinitionLoader& defLoader, const AssetPackConfig& assetConfig);
        Result operator()(Scene& scene, std::filesystem::path path);

    private:
        OptionalRef<ISceneDefinitionLoader> _defLoader;
        SceneArchive _archive;
    };

}