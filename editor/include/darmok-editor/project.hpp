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
        expected<bool, std::string> destroyEntity(EntityId entity) noexcept;

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

        template<typename T, typename Def = T::Definition>
        expected<void, std::string> removeSceneComponent() noexcept
        {
            if (!_sceneDefWrapper.removeSceneComponent<Def>())
            {
                return unexpected{ "failed to remove scene definition component" };
            }
            if (!_scene)
            {
                return unexpected<std::string>{"missing scene"};
            }
            auto removeResult = _scene->removeSceneComponent<T>();
            if (!removeResult)
            {
                return unexpected{ std::move(removeResult).error() };
            }
            if (!removeResult.value())
            {
                return unexpected{ "failed to remove scene component" };
            }
            return {};
        }

        template<typename T, typename Def = T::Definition>
        expected<void, std::string> removeEntityComponent(EntityId entityId) noexcept
        {
            if (!_sceneDefWrapper.removeComponent<Def>(entityId))
            {
                return unexpected{ "failed to remove entity component definition" };
            }
            if (!_scene)
            {
                return unexpected<std::string>{"missing scene"};
            }
            auto entity = getComponentLoadContext().getEntity(entityId);
            if (!_scene->removeComponent<T>(entity))
            {
                return unexpected{ "failed to remove entity component" };
            }
            return {};
        }

        template<typename T, typename Def = T::Definition>
        expected<void, std::string> removeCameraComponent(EntityId entityId) noexcept
        {
            if (auto camDef = _sceneDefWrapper.getComponent<Camera::Definition>(entityId))
            {
                CameraDefinitionWrapper camWrap{ *camDef };
                if (!camWrap.removeComponent<Def>())
                {
                    return unexpected{ "failed to remove camera component definition" };
                }
            }
            if (!_scene)
            {
                return unexpected<std::string>{"missing scene"};
            }
            auto entity = getComponentLoadContext().getEntity(entityId);
            if (auto cam = _scene->getComponent<Camera>(entity))
            {
                auto removeResult = cam->removeComponent<T>();
                if (!removeResult)
                {
                    return unexpected{ std::move(removeResult).error() };
                }
                if (!removeResult.value())
                {
                    return unexpected{ "failed to remove camera component" };
                }
            }
            return {};
        }

        template<typename T, typename Def = T::Definition>
        expected<void, std::string> updateSceneComponent(const Def& def) noexcept
        {
            if (!_scene)
            {
                return unexpected<std::string>{"missing scene"};
            }
            auto compResult = _scene->getOrAddSceneComponent<T>();
            if (!compResult)
            {
                return unexpected{ std::move(compResult).error() };
            }
            auto& comp = compResult.value().get();
            return SceneArchive::loadComponent(comp, def, getComponentLoadContext());
        }

        template<typename T, typename Def = T::Definition>
        expected<void, std::string> updateEntityComponent(EntityId entityId, const Def& def) noexcept
        {
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
        expected<void, std::string> updateCameraComponent(EntityId entityId, const Def& def) noexcept
        {
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
        std::string _loadError;
        std::filesystem::path _path;
        std::filesystem::path _exportPath;

        static const FileDialogOptions _dialogOptions;

        expected<void, std::string> configureEditorScene(Scene& scene) noexcept;

        expected<void, std::string> doResetScene() noexcept;
        expected<void, std::string> doExportScene(std::filesystem::path path) noexcept;
        expected<void, std::string> doSaveScene() noexcept;
        void clearPath() noexcept;
    };


}