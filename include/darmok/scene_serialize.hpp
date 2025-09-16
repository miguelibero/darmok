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
                        comps[static_cast<Entity>(entityId)] = std::move(asset);
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
        virtual IAssetContext& getAssets() = 0;
        virtual const Scene& getScene() const = 0;
        virtual Scene& getScene() = 0;

        virtual Entity getEntity(uint32_t id) const = 0;
    };

	using DataSceneDefinitionLoader = DataProtobufLoader<ISceneDefinitionLoader>;

    class Scene;
    class SceneImporterImpl;
    class AssetPack;
    class AssetPackConfig;

    struct ComponentRef final
    {
        Entity entity;
        uint32_t type;
    };

    class SceneImporter final
    {
    public:
        using Error = std::string;
		using Definition = protobuf::Scene;
        using Result = expected<Entity, Error>;
        SceneImporter(Scene& scene, const AssetPackConfig& assetConfig);
		~SceneImporter();
        Result operator()(const Definition& def);

		IComponentLoadContext& getComponentLoadContext() noexcept;
		AssetPack& getAssetPack() noexcept;
    private:
		std::unique_ptr<SceneImporterImpl> _impl;
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

    class SceneLoaderImpl;

    class DARMOK_EXPORT SceneLoader final : public ISceneLoader
    {
    public:
        SceneLoader(ISceneDefinitionLoader& defLoader, const AssetPackConfig& assetConfig);
        ~SceneLoader();
        Result operator()(Scene& scene, std::filesystem::path path);

    private:
        std::unique_ptr<SceneLoaderImpl> _impl;
    };

}