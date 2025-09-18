#pragma once

#include <darmok/protobuf/scene.pb.h>
#include <darmok/expected.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/asset_pack.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/optional_ref.hpp>

#include <string>
#include <optional>
#include <variant>
#include <functional>

namespace darmok
{
    class SceneArchiveImpl final : public IComponentLoadContext
    {
    public:
        using Result = SceneArchive::Result;
        using LoadFunction = SceneArchive::LoadFunction;
        using ComponentData = SceneArchive::ComponentData;
        using SceneDefinition = protobuf::Scene;

        SceneArchiveImpl();

        // IComponentLoadContext
        IAssetContext& getAssets() noexcept override;
        const Scene& getScene() const noexcept override;
        Scene& getScene() noexcept override;
        Entity getEntity(uint32_t entityId) const noexcept override;

        // SceneArchive
        expected<Entity, std::string> load(const Scene::Definition& sceneDef, Scene& scene) noexcept;
        void operator()(std::underlying_type_t<Entity>& count) noexcept;
        void operator()(Entity& entity) noexcept;
        
        void addError(std::string_view error) noexcept;
        ComponentData getComponentData() noexcept;
        void addLoad(LoadFunction&& func);
        void addPostLoad(LoadFunction&& func);
        Result beforeLoadComponent(uint32_t typeId) noexcept;
        Result afterLoadComponent(uint32_t typeId) noexcept;
        entt::continuous_loader& getLoader() noexcept;

		AssetPack& getAssetPack() noexcept;
        const AssetPack& getAssetPack() const noexcept;
        void setAssetPackConfig(AssetPackConfig assetConfig) noexcept;

    private:
        static Scene _emptyScene;
        static const SceneDefinition _emptySceneDef;
        OptionalRef<Scene> _scene;        
        OptionalRef<const SceneDefinition> _sceneDef;
        AssetPackConfig _assetConfig;
        mutable std::unique_ptr<AssetPack> _assetPack;
        entt::continuous_loader _loader;
        size_t _count;
        entt::id_type _typeId;
        std::vector<std::string> _errors;
        std::vector<LoadFunction> _postLoadFuncs;
        std::vector<LoadFunction> _loadFuncs;

        void createAssetPack() const noexcept;
    };
}