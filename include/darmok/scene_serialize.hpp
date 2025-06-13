#pragma once

#include <darmok/export.h>
#include <darmok/expected.hpp>
#include <darmok/loader.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/asset_pack.hpp>
#include <darmok/protobuf/scene.pb.h>

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace darmok
{
    class ConstSceneDefinitionWrapper
    {
    public:
        using Definition = protobuf::Scene;
        using Message = google::protobuf::Message;
        using Any = google::protobuf::Any;
        using AssetGroup = protobuf::AssetGroup;
        using RegistryComponents = protobuf::RegistryComponents;

        ConstSceneDefinitionWrapper(const Definition& def) noexcept;
        const Definition& get() const noexcept;

        std::vector<uint32_t> getRootEntities() const noexcept;
        OptionalRef<const RegistryComponents> getTypeComponents(uint32_t typeId) const noexcept;
        std::vector<std::reference_wrapper<const Any>> getComponents(uint32_t entityId) const noexcept;        
        OptionalRef<const Any> getComponent(uint32_t entityId, uint32_t typeId) const noexcept;

        OptionalRef<const AssetGroup> getAssetGroup(uint32_t typeId) const noexcept;
        std::vector<std::string> getAssetPaths(uint32_t typeId) const noexcept;
        std::unordered_map<std::string, OptionalRef<const Any>> getAssets(uint32_t typeId) const noexcept;
        OptionalRef<const Any> getAsset(uint32_t typeId, const std::string& path) const noexcept;

        template<typename T>
        std::unordered_map<uint32_t, T> getTypeComponents() const noexcept
        {
            std::unordered_map<uint32_t, T> comps;
            if (auto typeComps = getTypeComponents(protobuf::getTypeId<T>()))
            {
                for(auto& [entityId, any] : typeComps->components())
                {
                    T asset;
                    if (any.UnpackTo(&asset))
                    {
                        comps[entityId] = std::move(asset);
                    }
				}
            }
            return comps;
        }

        template<typename T>
        std::optional<T> getComponent(uint32_t entityId) const noexcept
        {
            if (auto any = getComponent(entityId, protobuf::getTypeId<T>()))
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
        std::vector<std::string> getAssetPaths() const noexcept
        {
            return getAssetPaths(protobuf::getTypeId<T>());
        }

        template<typename T>
        std::unordered_map<std::string, T> getAssets() const noexcept
        {
            std::unordered_map<std::string, T> assets;
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
        std::optional<T> getAsset(const std::string& path) const noexcept
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
    private:
        OptionalRef<const Definition> _def;
    };

	class SceneDefinitionWrapper final : public ConstSceneDefinitionWrapper
    {
    public:
        using Definition = protobuf::Scene;
        using Message = google::protobuf::Message;
        using Any = google::protobuf::Any;
        using AssetGroup = protobuf::AssetGroup;
        using RegistryComponents = protobuf::RegistryComponents;

        SceneDefinitionWrapper(Definition& def) noexcept;
        Definition& get() noexcept;

        uint32_t createEntity() noexcept;
        bool addAsset(std::string_view path, Message& asset) noexcept;
        bool addComponent(uint32_t entityId, Message& comp) noexcept;
        std::vector<std::reference_wrapper<Any>> getComponents(uint32_t entityId) noexcept;
        OptionalRef<RegistryComponents> getTypeComponents(uint32_t typeId) noexcept;
        OptionalRef<Any> getComponent(uint32_t entityId, uint32_t typeId) noexcept;

        OptionalRef<AssetGroup> getAssetGroup(uint32_t typeId) noexcept;
        std::unordered_map<std::string, OptionalRef<Any>> getAssets(uint32_t typeId) noexcept;
        OptionalRef<Any> getAsset(uint32_t typeId, const std::string& path);        

    private:
		OptionalRef<Definition> _def;
    };

    class DARMOK_EXPORT BX_NO_VTABLE ISceneDefinitionLoader : public ILoader<protobuf::Scene>
    {
    };

    class Scene;

    class DARMOK_EXPORT BX_NO_VTABLE IComponentLoadContext
    {
    public:
		virtual ~IComponentLoadContext() = default;
        virtual AssetPack& getAssets() = 0;
        virtual Entity getEntity(uint32_t id) const = 0;
        virtual const Scene& getScene() const = 0;
        virtual Scene& getScene() = 0;
    };

	using DataSceneDefinitionLoader = DataProtobufLoader<ISceneDefinitionLoader>;

    class Scene;
    class SceneImporterImpl;

    class SceneImporter final
    {
    public:
        using Error = std::string;
		using Definition = protobuf::Scene;
        using Result = expected<Entity, Error>;
        SceneImporter(const AssetPackFallbacks& assetPackFallbacks);
		~SceneImporter();
        Result operator()(Scene& scene, const Definition& def);
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

    class DARMOK_EXPORT SceneLoader final : public ISceneLoader
    {
    public:
        SceneLoader(ISceneDefinitionLoader& defLoader, const AssetPackFallbacks& assetPackFallbacks = {});
        Result operator()(Scene& scene, std::filesystem::path path);

    private:
		ISceneDefinitionLoader& _defLoader;
        SceneImporter _importer;
    };

}