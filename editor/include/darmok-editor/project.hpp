#pragma once

#include <darmok-editor/asset_fwd.hpp>
#include <darmok/scene.hpp>
#include <darmok/program_core.hpp>
#include <darmok/material.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/asset_pack.hpp>

#include <filesystem>
#include <unordered_map>
#include <unordered_set>

namespace darmok::editor
{
    class EditorProject final
    {
    public:
        EditorProject(App& app);

        expected<void, std::string> init(const ProgramCompilerConfig& progCompilerConfig);
        void shutdown();

        void saveScene(bool forceNewPath = false);
        void openScene();
        expected<void, std::string> reloadScene();
        void exportScene();
        void resetScene();
        void updateScene();

        expected<void, std::string> render();
        Entity addEntity(Entity parentEntity = entt::null);

        std::shared_ptr<Scene> getScene();
        std::shared_ptr<const Scene> getScene() const;
        SceneDefinitionWrapper& getSceneDefinition();
        const SceneDefinitionWrapper& getSceneDefinition() const;
        OptionalRef<Camera> getCamera();
        OptionalRef<const Camera> getCamera() const;
        IComponentLoadContext& getComponentLoadContext();
        AssetPack& getAssets();

    private:
        App& _app;
        OptionalRef<Camera> _cam;
        std::shared_ptr<Scene> _scene;
		std::optional<SceneImporter> _sceneImporter;
        bool _requestUpdateScene;

        using SceneDefinition = protobuf::Scene;
        SceneDefinition _sceneDef;
		SceneDefinitionWrapper _sceneWrapper;
		AssetPackConfig _assetPackConfig;
        std::optional<ProgramCompiler> _progCompiler;
        bool _requestReset;

        std::filesystem::path _path;
        std::filesystem::path _exportPath;

        static const std::vector<std::string> _dialogFilters;
        static const char* _confirmNewPopup;

        void configureEditorScene(Scene& scene);
        void configureDefaultScene(SceneDefinitionWrapper& scene);

        expected<Entity, std::string> doResetScene();
        expected<Entity, std::string> doUpdateScene();
        void doSaveScene();
    };


}