#include <darmok-editor/project.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include <darmok/texture.hpp>
#include <darmok/camera.hpp>
#include <darmok/stream.hpp>
#include <darmok/material.hpp>
#include <darmok/program.hpp>
#include <darmok/scene.hpp>
#include <darmok/varying.hpp>
#include <darmok/window.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/physics3d.hpp>

// default scene
#include <darmok/scene_serialize.hpp>
#include <darmok/glm_serialize.hpp>
#include <darmok/transform.hpp>
#include <darmok/environment.hpp>
#include <darmok/light.hpp>
#include <darmok/shadow.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/culling.hpp>
#include <darmok/shape.hpp>

namespace darmok::editor
{
    const FileDialogOptions EditorProject::_dialogOptions =
    {
        .filters = { "*.dpj", "*.dpj.xml", "*.dpj.json" },
		.filterDesc = "Darmok Project Files"
    };

    EditorProject::EditorProject(App& app) noexcept
        : _app{ app }
        , _requestReset{ false }
        , _sceneDefWrapper{ _sceneDef }
        , _sceneLoader{ }
    {
        _sceneLoader.setAssetPackConfig({
            .fallback = app.getAssets()
        });
    }

    expected<void, std::string> EditorProject::init(const ProgramCompilerConfig& progCompilerConfig) noexcept
    {
        auto result = doResetScene();
        if (!result)
        {
			return unexpected{ result.error() };
        }
        _sceneLoader.setAssetPackConfig({
            .programCompilerConfig = progCompilerConfig,
            .fallback = _app.getAssets(),
        });

        _requestReset = false;
        return {};
    }

    expected<void, std::string> EditorProject::shutdown() noexcept
    {
        clearPath();
        _requestReset = false;
        return {};
    }

    expected<void, std::string> EditorProject::doSaveScene() noexcept
    {
        if (_path.empty())
        {
            return {};
        }
        return protobuf::write(_sceneDef, _path);
    }

    expected<void, std::string> EditorProject::saveScene(bool forceNewPath) noexcept
    {
        if (_path.empty() || forceNewPath)
        {
            auto dialogCallback = [this](auto& result) -> expected<void, std::string>
                {
                    if (!result.empty())
                    {
                        clearPath();
                        _path = result.front();
                    }
                    return doSaveScene();
                };

            auto options = _dialogOptions;
            options.type = FileDialogType::Save;
            options.title = "Save Project";
            options.defaultPath = _path;
            _app.getWindow().openFileDialog(std::move(options), std::move(dialogCallback));
            return {};
        }
        return doSaveScene();
    }

    expected<void, std::string> EditorProject::doExportScene(std::filesystem::path path) noexcept
    {
        SceneDefinitionCompiler compiler{};
        auto sceneDef = _sceneDef;
        DARMOK_TRY_PREFIX(compiler(sceneDef), "compiling scene: ");
        DARMOK_TRY_PREFIX(protobuf::write(sceneDef, path), "writing scene: ");
        return {};
    }

    expected<void, std::string> EditorProject::exportScene() noexcept
    {
        auto dialogCallback = [this](auto& result) -> expected<void, std::string>
            {
                if (result.empty())
                {
                    return {};
                }
                return doExportScene(result.front());
            };

        FileDialogOptions options;
        options.filters = { "*.dsc", "*.dsc.xml", "*.dsc.json" };
        options.filterDesc = "Darmok Scene Files";
        options.type = FileDialogType::Save;
        options.title = "Export Scene";
        options.defaultPath = _path;
        _app.getWindow().openFileDialog(std::move(options), std::move(dialogCallback));
        return {};
    }

    expected<void, std::string> EditorProject::render() noexcept
    {
        const char* confirmNewPopup = "Confirm New Project";
        const char* loadErrorPopup = "Scene Load Error";

        if (_requestReset)
        {
            _requestReset = false;
            ImGui::OpenPopup(confirmNewPopup);
        }
        if (!_loadError.empty())
        {
            ImGui::OpenPopup(loadErrorPopup);
        }
        if (ImguiUtils::drawErrorPopup(loadErrorPopup, _loadError.c_str()))
        {
            _loadError.clear();
        }
        auto action = ImguiUtils::drawConfirmPopup(confirmNewPopup, "Are you sure you want to create a new project?");
        if (action == ConfirmPopupAction::Ok)
        {
            DARMOK_TRY(doResetScene());
        }
        return {};
    }

    expected<EntityId, std::string> EditorProject::addEntity(EntityId parentEntityId) noexcept
    {
        auto entityId = _sceneDefWrapper.createEntity();
        auto transDef = Transform::createDefinition();
        transDef.set_name("New Entity");
        transDef.set_parent(parentEntityId);
        _sceneDefWrapper.setComponent(entityId, transDef);

        if (_scene)
        {
            DARMOK_TRY(_sceneLoader(_sceneDef, *_scene));
        }
        return entityId;
    }

    expected<bool, std::string> EditorProject::destroyEntity(EntityId entityId) noexcept
    {
        if (!_sceneDefWrapper.destroyEntity(entityId))
        {
            return false;
        }
        if (!_scene)
        {
            return unexpected<std::string>{"missing scene"};
        }
        auto entity = getComponentLoadContext().getEntity(entityId);
        _scene->destroyEntity(entity);
        return true;
    }

    void EditorProject::requestResetScene() noexcept
    {
        _requestReset = true;
    }

    void EditorProject::clearPath() noexcept
    {
        if (_path.has_parent_path())
        {
            _app.removeAssetsRootPath(_path.parent_path());
        }
        _path.clear();
    }

    expected<void, std::string> EditorProject::doResetScene() noexcept
    {
        _requestReset = false;
        clearPath();

        OptionalRef<SceneAppComponent> compRef;
        DARMOK_TRY_VALUE(compRef, _app.getOrAddComponent<SceneAppComponent>());
        _sceneDef = SceneLoader::createDefinition(false);
        _scene = compRef->getScene();
        _scene->setPaused(true);
        _scene->destroyEntitiesImmediate();
        DARMOK_TRY(configureEditorScene(*_scene));
        return updateScene();
    }

    expected<void, std::string> EditorProject::updateScene() noexcept
    {
        if (_path.has_parent_path())
        {
            _app.addAssetsRootPath(_path.parent_path());
        }
        auto loadResult = _sceneLoader(_sceneDef, *_scene);
        if (!loadResult)
        {
            _loadError = loadResult.error();
            return unexpected{ std::move(loadResult).error() };
        }
        DARMOK_TRY(_app.getOrAddComponent<SceneAppComponent>());

        for (auto& entity : _scene->getComponents<Camera>())
        {
            if (auto cam = _scene->getComponent<Camera>(entity))
            {
                DARMOK_TRY(cam->setRenderOutputSize(glm::uvec2{ 0 }));
            }
        }
        _app.requestRenderReset();
        return {};
    }

    expected<void, std::string> EditorProject::reloadScene() noexcept
    {
        if (_path.empty())
        {
            return {};
        }
        DARMOK_TRY(protobuf::read(_sceneDef, _path));
        return updateScene();
    }

    expected<void, std::string> EditorProject::openScene() noexcept
    {
		auto dialogCallback = [this](auto& dialogResult) -> expected<void, std::string>
        {
            if (dialogResult.empty())
            {
                return {};
            }
            std::filesystem::path path{ dialogResult[0] };
            if (!std::filesystem::exists(path))
            {
                return unexpected<std::string>{"path does not exist"};
            }
            clearPath();
            _path = path;
            return reloadScene();
		};

		auto options = _dialogOptions;
        options.type = FileDialogType::Open;
        options.title = "Open Project";
        options.defaultPath = "./";
        _app.getWindow().openFileDialog(std::move(options), std::move(dialogCallback));
        return {};
    }

    std::shared_ptr<Scene> EditorProject::getScene() noexcept
    {
        return _scene;
    }

    std::shared_ptr<const Scene> EditorProject::getScene() const noexcept
    {
        return _scene;
    }

    SceneDefinitionWrapper& EditorProject::getSceneDefinition() noexcept
    {
        return _sceneDefWrapper;
    }

    const SceneDefinitionWrapper& EditorProject::getSceneDefinition() const noexcept
    {
		return _sceneDefWrapper;
    }

    OptionalRef<Camera> EditorProject::getCamera() noexcept
    {
        return _cam;
    }

    OptionalRef<const Camera> EditorProject::getCamera() const noexcept
    {
        return _cam;
    }

    IComponentLoadContext& EditorProject::getComponentLoadContext() noexcept
    {
        return _sceneLoader.getComponentLoadContext();
    }

    const IComponentLoadContext& EditorProject::getComponentLoadContext() const noexcept
    {
        return _sceneLoader.getComponentLoadContext();
    }

    AssetPack& EditorProject::getAssets() noexcept
    {
		return _sceneLoader.getAssetPack();
    }

    const AssetPack& EditorProject::getAssets() const noexcept
    {
        return _sceneLoader.getAssetPack();
    }

    expected<void, std::string> EditorProject::configureEditorScene(Scene& scene) noexcept
    {
        auto camEntity = scene.createEntity();
        auto& cam = scene.addComponent<Camera>(camEntity);

        scene.addComponent<Transform>(camEntity)
            .setPosition(glm::vec3{ 2.f, 1.f, -2.f })
            .lookAt(glm::vec3{ 0 })
            .setName("Editor Camera");
        cam.setPerspective(glm::radians(60.f), 0.3f, 1000.f);

        std::shared_ptr<Texture> skybox;
        DARMOK_TRY_VALUE(skybox, _app.getAssets().getTextureLoader()("cubemap.ktx"));
        DARMOK_TRY(cam.addComponent<SkyboxRenderer>(skybox));
        DARMOK_TRY(cam.addComponent<GridRenderer>());
        DARMOK_TRY(cam.addComponent<LightingRenderComponent>());
        DARMOK_TRY(cam.addComponent<ShadowRenderer>());
        DARMOK_TRY(cam.addComponent<ForwardRenderer>());
        DARMOK_TRY(cam.addComponent<FrustumCuller>());
        _cam = cam;
        return {};
    }

    expected<void, std::string> EditorProject::updatePrefab(EntityId entityId, const std::string& scenePath) noexcept
    {
        return updateScene();
    }

    expected<void, std::string> EditorProject::removePrefab(EntityId entityId) noexcept
    {
        return updateScene();
    }

    std::filesystem::path EditorProject::addAsset(const std::filesystem::path& path, const Message& asset) noexcept
    {
		auto fpath = _sceneDefWrapper.addAsset(path, asset);
        _sceneLoader.reload();
        return fpath;
    }
}