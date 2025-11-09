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

    EditorProject::EditorProject(App& app)
        : _app{ app }
        , _requestReset{ false }
        , _requestUpdateScene{ false }
        , _sceneWrapper{ _sceneDef }
    {
    }

    expected<void, std::string> EditorProject::init(const ProgramCompilerConfig& progCompilerConfig)
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
        _requestUpdateScene = false;
        return {};
    }

    void EditorProject::shutdown()
    {
        clearPath();
        _requestReset = false;
        _requestUpdateScene = false;
    }

    void EditorProject::doSaveScene()
    {
        if (_path.empty())
        {
            return;
        }
        auto result = protobuf::write(_sceneDef, _path);
        if (!result)
        {
            StreamUtils::log("error writing scene: " + result.error());
        }
    }

    void EditorProject::saveScene(bool forceNewPath)
    {
        if (_path.empty() || forceNewPath)
        {
            auto dialogCallback = [this](auto& result)
            {
                if (!result.empty())
                {
                    clearPath();
                    _path = result.front();
                }
                doSaveScene();
            };

			auto options = _dialogOptions;
			options.type = FileDialogType::Save;
			options.title = "Save Project";
			options.defaultPath = _path;
            _app.getWindow().openFileDialog(std::move(options), std::move(dialogCallback));
            return;
        }
        doSaveScene();
    }

    expected<void, std::string> EditorProject::doExportScene(std::filesystem::path path)
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

    void EditorProject::exportScene()
    {
        auto dialogCallback = [this](auto& result)
        {
            if (result.empty())
            {
                return;
            }
            auto exportResult = doExportScene(result.front());
            if (!exportResult)
            {
                StreamUtils::log(exportResult.error());
            }
        };

        FileDialogOptions options;
        options.filters = { "*.dsc", "*.dsc.xml", "*.dsc.json" };
        options.filterDesc = "Darmok Scene Files";
        options.type = FileDialogType::Save;
        options.title = "Export Scene";
        options.defaultPath = _path;
        _app.getWindow().openFileDialog(std::move(options), std::move(dialogCallback));
    }

    expected<void, std::string> EditorProject::render()
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
        if (_requestUpdateScene)
        {
            auto result = doUpdateScene();
            if (!result)
            {
                return unexpected{ result.error() };
            }
        }
        return {};
    }

    void EditorProject::updateScene()
    {
        _requestUpdateScene = true;
    }

    EntityId EditorProject::addEntity(EntityId parentEntity)
    {
        auto entity = _sceneWrapper.createEntity();
        auto trans = Transform::createDefinition();
        trans.set_name("New Entity");
        if (parentEntity != entt::null)
        {
            trans.set_parent(entt::to_integral(parentEntity));
        }
        _sceneWrapper.setComponent(entity, trans);
        updateScene();
        return entity;
    }

    const char* EditorProject::_confirmNewPopup = "Confirm New Project";

    void EditorProject::resetScene()
    {
        _requestReset = true;
    }

    void EditorProject::clearPath()
    {
        if (_path.has_parent_path())
        {
            _app.removeAssetsRootPath(_path.parent_path());
        }
        _path.clear();
    }

    expected<void, std::string> EditorProject::doResetScene()
    {
        _requestReset = false;
        clearPath();

        auto& scenes = _app.getOrAddComponent<SceneAppComponent>();
        _scene = scenes.getScene();

        configureDefaultScene(_sceneWrapper);
        
        _scene->destroyEntitiesImmediate();
        configureEditorScene(*_scene);
        return doUpdateScene();
    }

    expected<void, std::string> EditorProject::doUpdateScene()
    {
        _requestUpdateScene = false;

        if (_path.has_parent_path())
        {
            _app.addAssetsRootPath(_path.parent_path());
        }

        auto result = _sceneLoader(_sceneDef, *_scene);
        if (result)
        {
            _app.getOrAddComponent<SceneAppComponent>().update(0.f);
            return {};
        }
        return unexpected{ result.error() };
    }

    expected<void, std::string> EditorProject::reloadScene()
    {
        if (_path.empty())
        {
            return {};
        }
        auto result = protobuf::read(_sceneDef, _path);
        if (!result)
        {
            StreamUtils::log("error reading scene: " + result.error());
            return result;
        }
        updateScene();
        return {};
    }

    void EditorProject::openScene()
    {
        auto dialogCallback = [this](auto& dialogResult)
        {
            if (dialogResult.empty())
            {
                return;
            }
            std::filesystem::path path{ dialogResult[0] };
            if (!std::filesystem::exists(path))
            {
                return;
            }
            clearPath();
            _path = path;
            (void)reloadScene();
		};

		auto options = _dialogOptions;
        options.type = FileDialogType::Open;
        options.title = "Open Project";
        options.defaultPath = "./";
        _app.getWindow().openFileDialog(std::move(options), std::move(dialogCallback));
    }

    std::shared_ptr<Scene> EditorProject::getScene()
    {
        return _scene;
    }

    std::shared_ptr<const Scene> EditorProject::getScene() const
    {
        return _scene;
    }

    SceneDefinitionWrapper& EditorProject::getSceneDefinition()
    {
        return _sceneWrapper;
    }

    const SceneDefinitionWrapper& EditorProject::getSceneDefinition() const
    {
		return _sceneWrapper;
    }

    OptionalRef<Camera> EditorProject::getCamera()
    {
        return _cam;
    }

    OptionalRef<const Camera> EditorProject::getCamera() const
    {
        return _cam;
    }

    IComponentLoadContext& EditorProject::getComponentLoadContext()
    {
        return _sceneLoader.getComponentLoadContext();
    }

    const IComponentLoadContext& EditorProject::getComponentLoadContext() const
    {
        return _sceneLoader.getComponentLoadContext();
    }

    AssetPack& EditorProject::getAssets()
    {
		return _sceneLoader.getAssetPack();
    }

    void EditorProject::configureEditorScene(Scene& scene)
    {
        static const std::string name = "Editor Camera";
        auto camEntity = scene.createEntity();
        auto& cam = scene.addComponent<Camera>(camEntity);

        scene.addComponent<Transform>(camEntity)
            .setPosition(glm::vec3{ 2.f, 1.f, -2.f })
            .lookAt(glm::vec3{ 0 })
            .setName(name);
        cam.setPerspective(glm::radians(60.f), 0.3f, 1000.f);
        if (auto skyboxTexResult = _app.getAssets().getTextureLoader()("cubemap.ktx"))
        {
            cam.addComponent<SkyboxRenderer>(skyboxTexResult.value());
        }
        cam.addComponent<GridRenderer>();
        cam.addComponent<LightingRenderComponent>();
        ShadowRendererConfig shadowConfig;
        shadowConfig.cascadeAmount = 3;
        cam.addComponent<ShadowRenderer>(shadowConfig);
        cam.addComponent<ForwardRenderer>();
        cam.addComponent<FrustumCuller>();
        _cam = cam;
    }

    void EditorProject::configureDefaultScene(SceneDefinitionWrapper& scene)
    {
        scene.setName("Scene");

        ShadowRendererConfig shadowConfig;
        shadowConfig.cascadeAmount = 3;

        auto camEntity = scene.createEntity();

        auto cam = Camera::createDefinition();
        scene.setComponent(camEntity, cam);

        auto camTrans = Transform::createDefinition();
        camTrans.set_name("Main Camera");
        *camTrans.mutable_position() = protobuf::convert(glm::vec3{ 0.f, 1.f, -10.f });
        scene.setComponent(camEntity, camTrans);

        auto lightsEntity = scene.createEntity();
        auto lightsTrans = Transform::createDefinition();
        lightsTrans.set_name("Lighting");
        scene.setComponent(lightsEntity, lightsTrans);

        auto ambLightEntity = scene.createEntity();
        auto ambLight = AmbientLight::createDefinition();
        ambLight.set_intensity(0.2f);
        scene.setComponent(ambLightEntity, ambLight);
        auto ambLightTrans = Transform::createDefinition();
        ambLightTrans.set_name("Ambient Light");
        ambLightTrans.set_parent(entt::to_integral(lightsEntity));
        scene.setComponent(ambLightEntity, ambLightTrans);

        auto dirLightEntity = scene.createEntity();
        auto dirLight = DirectionalLight::createDefinition();
        dirLight.set_intensity(3.f);
        scene.setComponent(dirLightEntity, dirLight);
        auto dirLightTrans = Transform::createDefinition();
        dirLightTrans.set_name("Directional Light");
        dirLightTrans.set_parent(entt::to_integral(lightsEntity));
        *dirLightTrans.mutable_position() = protobuf::convert(glm::vec3{ 0.f, 3.f, 0.f });
        auto rot = glm::quat{ glm::radians(glm::vec3{50.f, -30.f, 0.f}) };
        *dirLightTrans.mutable_rotation() = protobuf::convert(rot);
        scene.setComponent(dirLightEntity, dirLightTrans);

        auto mesh = Mesh::createSource();
		mesh.set_name("Default Mesh");
        std::string meshPath = "default/mesh";
        scene.setAsset(meshPath, mesh);

        auto mat = Material::createDefinition();
        mat.set_name("Default");
        std::string matPath = "default/material";
        scene.setAsset(matPath, mat);

        auto renderableEntity = scene.createEntity();
        auto renderable = Renderable::createDefinition();
        renderable.set_mesh_path(meshPath);
        renderable.set_material_path(matPath);
        scene.setComponent(renderableEntity, renderable);

        auto renderableTrans = Transform::createDefinition();
        renderableTrans.set_name("Sphere");
        scene.setComponent(renderableEntity, renderableTrans);
    }
}