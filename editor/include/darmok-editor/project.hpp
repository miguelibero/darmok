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

        void init(const ProgramCompilerConfig& progCompilerConfig);
        void shutdown();

        void save(bool forceNewPath = false);
        void open();
        void exportScene();
        void reset();
        void render();
        void updateScene();

        std::shared_ptr<Scene> getScene();
        SceneDefinitionWrapper& getSceneDefinition();
        OptionalRef<Camera> getCamera();
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
        std::optional<AssetPack> _assets;
        std::optional<ProgramCompiler> _progCompiler;
        bool _requestReset;

        std::filesystem::path _path;
        std::filesystem::path _exportPath;
        static const std::vector<std::string> _dialogFilters;
        static const char* _confirmNewPopup;

        void configureEditorScene(Scene& scene);
        void configureDefaultScene(SceneDefinitionWrapper& scene);

        void doReset();
		void doUpdateScene();
    };
}