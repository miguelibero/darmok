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
        auto compileResult = compiler(sceneDef);
        if (!compileResult)
        {
            return unexpected{ "failed to compile scene: " + compileResult.error() };
        }
        auto writeResult = protobuf::write(sceneDef, path);
        if (!writeResult)
        {
            return unexpected{ "failed to write scene: " + writeResult.error() };
        }
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
        if (_requestReset)
        {
            _requestReset = false;
            ImGui::OpenPopup(_confirmNewPopup);
        }
        auto action = ImguiUtils::drawConfirmPopup(_confirmNewPopup, "Are you sure you want to create a new project?");
        if (action == ConfirmPopupAction::Ok)
        {
            auto result = doResetScene();
            if(!result)
            {
                return unexpected{ result.error() };
			}
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
        auto entity = _scene->createEntity();
        auto& trans = _scene->addComponent<Transform>(entity);
        auto loadResult = SceneArchive::loadComponent(trans, transDef, getComponentLoadContext());
        if (!loadResult)
        {
            return unexpected{ std::move(loadResult).error() };
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

    const char* EditorProject::_confirmNewPopup = "Confirm New Project";

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

        auto compResult = _app.getOrAddComponent<SceneAppComponent>();
        if (!compResult)
        {
            return unexpected{ std::move(compResult).error() };
        }
        _sceneDef = SceneLoader::createDefinition(false);
        _scene = compResult.value().get().getScene();
        _scene->setPaused(true);
        _scene->destroyEntitiesImmediate();
        auto result = configureEditorScene(*_scene);
        if (!result)
        {
            return result;
        }
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
            return unexpected{ std::move(loadResult).error() };
        }
        auto compResult = _app.getOrAddComponent<SceneAppComponent>();
        if (!compResult)
        {
            return unexpected{ std::move(compResult).error() };
        }

        for (auto& entity : _scene->getComponents<Camera>())
        {
            if (auto cam = _scene->getComponent<Camera>(entity))
            {
                auto camResult = cam->setRenderOutputSize(glm::uvec2{ 0 });
                if (!camResult)
                {
                    return unexpected{ std::move(camResult).error() };
                }
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
        auto result = protobuf::read(_sceneDef, _path);
        if (!result)
        {
            return result;
        }
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

    expected<void, std::string> EditorProject::configureEditorScene(Scene& scene) noexcept
    {
        auto camEntity = scene.createEntity();
        auto& cam = scene.addComponent<Camera>(camEntity);

        scene.addComponent<Transform>(camEntity)
            .setPosition(glm::vec3{ 2.f, 1.f, -2.f })
            .lookAt(glm::vec3{ 0 })
            .setName("Editor Camera");
        cam.setPerspective(glm::radians(60.f), 0.3f, 1000.f);
        {
            auto texResult = _app.getAssets().getTextureLoader()("cubemap.ktx");
            if (!texResult)
            {
                return unexpected{ std::move(texResult).error() };
            }
            auto result = cam.addComponent<SkyboxRenderer>(texResult.value());
            if (!result)
            {
                return unexpected{ std::move(result).error() };
            }
        }
        {
            auto result = cam.addComponent<GridRenderer>();
            if (!result)
            {
                return unexpected{ std::move(result).error() };
            }
        }
        {
            auto result = cam.addComponent<LightingRenderComponent>();
            if (!result)
            {
                return unexpected{ std::move(result).error() };
            }
        }
        {
            auto result = cam.addComponent<ShadowRenderer>();
            if (!result)
            {
                return unexpected{ std::move(result).error() };
            }
        }
        {
            auto result = cam.addComponent<ForwardRenderer>();
            if (!result)
            {
                return unexpected{ std::move(result).error() };
            }
        }
        {
            auto result = cam.addComponent<FrustumCuller>();
            if (!result)
            {
                return unexpected{ std::move(result).error() };
            }
        }
        _cam = cam;
        return {};
    }
}