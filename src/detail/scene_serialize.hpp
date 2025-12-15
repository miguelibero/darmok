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
    class SceneLoaderImpl final : public IComponentLoadContext
    {
    public:
        using Result = SceneLoader::Result;
        using EntityResult = SceneLoader::EntityResult;
        using LoadFunction = SceneLoader::LoadFunction;
        using ComponentData = SceneLoader::ComponentData;
        using Message = SceneLoader::Message;
		using Any = SceneLoader::Any;
        using SceneDefinition = protobuf::Scene;
        using ComponentListener = SceneLoader::ComponentListener;

        SceneLoaderImpl(App& app) noexcept;

        // IComponentLoadContext
        IAssetContext& getAssets() noexcept override;
        const App& getApp() const noexcept override;
        App& getApp() noexcept override;
        const Scene& getScene() const noexcept override;
        Scene& getScene() noexcept override;
        Entity getEntity(EntityId entityId) const noexcept override;

        // SceneLoader
        EntityResult load(const Scene::Definition& sceneDef, Scene& scene) noexcept;
        void operator()(std::underlying_type_t<Entity>& count) noexcept;
        void operator()(Entity& entity) noexcept;
        

		const SceneDefinition& getSceneDefinition() const noexcept;
        void addError(std::string_view error) noexcept;
        ComponentData getComponentData() noexcept;
        void addLoad(LoadFunction&& func);
        void addPostLoad(LoadFunction&& func);
        Result beforeLoadComponent(IdType typeId) noexcept;
        Result afterLoadComponent(IdType typeId) noexcept;
        entt::continuous_loader& getLoader() noexcept;

		AssetPack& getAssetPack() noexcept;
        const AssetPack& getAssetPack() const noexcept;
        void setParent(Entity entity) noexcept;
        void setAssetPackConfig(AssetPackConfig assetConfig) noexcept;
        void addComponentListener(ComponentListener func) noexcept;
        void clearComponentListeners() noexcept;
        Result callComponentListeners(const Any& compAny, Entity entity) noexcept;

    private:
        static Scene _emptyScene;
        static const SceneDefinition _emptySceneDef;
        App& _app;
        OptionalRef<Scene> _scene;     
        OptionalRef<const SceneDefinition> _sceneDef;
        Entity _parentEntity;
        std::vector<ComponentListener> _compListeners;
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