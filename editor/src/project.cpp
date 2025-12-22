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
        , _requestUpdateScene{ false }
        , _sceneWrapper{ _sceneDef }
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
        _requestUpdateScene = false;
        return {};
    }

    expected<void, std::string> EditorProject::shutdown() noexcept
    {
        clearPath();
        _requestReset = false;
        _requestUpdateScene = false;
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

    void EditorProject::requestSceneUpdate() noexcept
    {
        _requestUpdateScene = true;
    }

    expected<EntityId, std::string> EditorProject::addEntity(EntityId parentEntity) noexcept
    {
        auto entity = _sceneWrapper.createEntity();
        auto trans = Transform::createDefinition();
        trans.set_name("New Entity");
        if (parentEntity != entt::null)
        {
            trans.set_parent(entt::to_integral(parentEntity));
        }
        _sceneWrapper.setComponent(entity, trans);
        requestSceneUpdate();
        return entity;
    }

    const char* EditorProject::_confirmNewPopup = "Confirm New Project";

    expected<void, std::string> EditorProject::resetScene() noexcept
    {
        _requestReset = true;
        return {};
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
        _scene = compResult.value().get().getScene();
        _scene->setPaused(true);

        auto result = configureDefaultScene(_sceneWrapper);
        if (!result)
        {
            return result;
        }

        _scene->destroyEntitiesImmediate();
        result = configureEditorScene(*_scene);
        if (!result)
        {
            return result;
        }
        return doUpdateScene();
    }

    expected<void, std::string> EditorProject::doUpdateScene() noexcept
    {
        _requestUpdateScene = false;

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
        return compResult.value().get().update(0.f);
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
        requestSceneUpdate();
        return {};
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
        return _sceneWrapper;
    }

    const SceneDefinitionWrapper& EditorProject::getSceneDefinition() const noexcept
    {
		return _sceneWrapper;
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

    expected<void, std::string> EditorProject::configureDefaultScene(SceneDefinitionWrapper& scene) noexcept
    {
        scene.setName("Scene");
        {
            auto def = SkeletalAnimationSceneComponent::createDefinition();
            scene.setSceneComponent(def);
        }

        auto camEntity = scene.createEntity();

        auto cam = Camera::createDefinition();
        scene.setComponent(camEntity, cam);
        CameraDefinitionWrapper camWrapper{ cam };

        {
            auto def = LightingRenderComponent::createDefinition();
            camWrapper.setComponent(def);
        }
        {
            auto def = ShadowRenderer::createDefinition();
            camWrapper.setComponent(def);
        }
        {
            auto def = ForwardRenderer::createDefinition();
            camWrapper.setComponent(def);
        }
        {
            auto def = SkeletalAnimationRenderComponent::createDefinition();
        }


        auto camTrans = Transform::createDefinition();
        camTrans.set_name("Main Camera");
        *camTrans.mutable_position() = convert<protobuf::Vec3>(glm::vec3{ 0.f, 1.f, -10.f });
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
        *dirLightTrans.mutable_position() = convert<protobuf::Vec3>(glm::vec3{ 0.f, 3.f, 0.f });
        auto rot = glm::quat{ glm::radians(glm::vec3{50.f, -30.f, 0.f}) };
        *dirLightTrans.mutable_rotation() = convert<protobuf::Quat>(rot);
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

        return {};
    }
}