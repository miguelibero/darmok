#pragma once

#include <darmok-editor/asset_fwd.hpp>
#include <darmok/scene.hpp>
#include <darmok/program_core.hpp>
#include <darmok/material.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/asset_pack.hpp>
#include <darmok/window.hpp>

#include <filesystem>
#include <unordered_map>
#include <unordered_set>

namespace darmok::editor
{
    class EditorProject final
    {
    public:
        EditorProject(App& app) noexcept;

        expected<void, std::string> init(const ProgramCompilerConfig& progCompilerConfig) noexcept;
        expected<void, std::string> shutdown() noexcept;

        expected<void, std::string> saveScene(bool forceNewPath = false) noexcept;
        expected<void, std::string> openScene() noexcept;
        expected<void, std::string> reloadScene() noexcept;
        expected<void, std::string> exportScene() noexcept;
        expected<void, std::string> updateScene() noexcept;
        void requestResetScene() noexcept;

        expected<void, std::string> render() noexcept;
        expected<EntityId, std::string> addEntity(EntityId parentEntity = 0) noexcept;

        template<typename T, typename Def = T::Definition>
        expected<void, std::string> addSceneComponent(const Def& def) noexcept
        {
            _sceneDefWrapper.setSceneComponent(def);
            if (!_scene)
            {
                return unexpected<std::string>{"missing scene"};
            }
            auto compResult = _scene->getOrAddSceneComponent<T>();
            if (!compResult)
            {
                return unexpected{ std::move(compResult).error() };
            }
            return SceneArchive::loadComponent(compResult.value().get(), def, getComponentLoadContext());
        }
        
        template<typename T, typename Def = T::Definition>
        expected<void, std::string> addEntityComponent(EntityId entityId, const Def& def) noexcept
        {
            _sceneDefWrapper.setComponent(entityId, def);
            if (!_scene)
            {
                return unexpected<std::string>{"missing scene"};
            }
            auto& context = getComponentLoadContext();
            auto entity = context.getEntity(entityId);
            auto& comp = _scene->getOrAddComponent<T>(entity);
            return SceneArchive::loadComponent(comp, def, context);
        }
        
        template<typename T, typename Def = T::Definition>
        expected<void, std::string> addCameraComponent(EntityId entityId, const Def& def) noexcept
        {
            auto camDef = _sceneDefWrapper.getOrAddComponent<Camera::Definition>(entityId);
            CameraDefinitionWrapper camDefWrapper{ camDef };
            camDefWrapper.setComponent(def);
            _sceneDefWrapper.setComponent(entityId, camDef);
            if (!_scene)
            {
                return unexpected<std::string>{"missing scene"};
            }
            auto& context = getComponentLoadContext();
            auto entity = context.getEntity(entityId);
            auto& cam = _scene->getOrAddComponent<Camera>(entity);
            auto compResult = cam.getOrAddComponent<T>();
            if (!compResult)
            {
                return unexpected{ std::move(compResult).error() };
            }
            return SceneArchive::loadComponent(compResult.value().get(), def, context);
        }

        std::shared_ptr<Scene> getScene() noexcept;
        std::shared_ptr<const Scene> getScene() const noexcept;
        SceneDefinitionWrapper& getSceneDefinition() noexcept;
        const SceneDefinitionWrapper& getSceneDefinition() const noexcept;
        OptionalRef<Camera> getCamera() noexcept;
        OptionalRef<const Camera> getCamera() const noexcept;
        IComponentLoadContext& getComponentLoadContext() noexcept;
        const IComponentLoadContext& getComponentLoadContext() const noexcept;
        AssetPack& getAssets() noexcept;

    private:
        App& _app;
        OptionalRef<Camera> _cam;
        std::shared_ptr<Scene> _scene;
		SceneLoader _sceneLoader;

        using SceneDefinition = protobuf::Scene;
        SceneDefinition _sceneDef;
		SceneDefinitionWrapper _sceneDefWrapper;
        bool _requestReset;

        std::filesystem::path _path;
        std::filesystem::path _exportPath;

        static const FileDialogOptions _dialogOptions;
        static const char* _confirmNewPopup;

        expected<void, std::string> configureEditorScene(Scene& scene) noexcept;
        expected<void, std::string> configureDefaultScene(SceneDefinitionWrapper& scene) noexcept;

        expected<void, std::string> doResetScene() noexcept;
        expected<void, std::string> doExportScene(std::filesystem::path path) noexcept;
        expected<void, std::string> doSaveScene() noexcept;
        void clearPath() noexcept;
    };


}