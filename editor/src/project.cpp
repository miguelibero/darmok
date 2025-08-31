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
#include <darmok/protobuf.hpp>

#include <portable-file-dialogs.h>

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
    const std::vector<std::string> EditorProject::_dialogFilters =
    {
        "Darmok Project Files", "*.dpj",
        "Darmok Project XML Files", "*.dpj.xml",
        "Darmok Project json Files", "*.dpj.json",
        "All Files", "*"
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
        auto result = doReset();
        if (!result)
        {
			return unexpected{ result.error() };
        }
        _progCompiler.emplace(progCompilerConfig);
        _requestReset = false;
        _requestUpdateScene = false;
        return {};
    }

    void EditorProject::shutdown()
    {
        _path.clear();
        _requestReset = false;
        _requestUpdateScene = false;
    }

    void EditorProject::save(bool forceNewPath)
    {
        if (!_scene)
        {
            return;
        }
        if (_path.empty() || forceNewPath)
        {
            std::string initialPath{ "." };
            if (!_path.empty() && std::filesystem::exists(_path))
            {
                initialPath = _path.string();
            }
            auto dialog = pfd::save_file("Save Project", initialPath,
                _dialogFilters, pfd::opt::force_path);

            while (!dialog.ready(1000))
            {
                StreamUtils::logDebug("waiting for save dialog...");
            }
            _path = dialog.result();
        }
        if (_path.empty())
        {
            return;
        }
        auto result = protobuf::write(_sceneDef, _path);
        if (!result)
        {
            StreamUtils::logDebug("error writing scene: " + result.error());
        }
    }

    void EditorProject::exportScene()
    {
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
            auto result = doReset();
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

    Entity EditorProject::addEntity(Entity parentEntity)
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

    void EditorProject::reset()
    {
        _requestReset = true;
    }

    expected<Entity, std::string> EditorProject::doReset()
    {
        _requestReset = false;
        _path.clear();

        auto& scenes = _app.getOrAddComponent<SceneAppComponent>();
        _scene = scenes.getScene();

        configureDefaultScene(_sceneWrapper);
		_sceneImporter.emplace(*_scene, _assetPackConfig);
        
        _scene->destroyEntitiesImmediate();
        configureEditorScene(*_scene);
        return doUpdateScene();
    }

    expected<Entity, std::string> EditorProject::doUpdateScene()
    {
        _requestUpdateScene = false;
        if (!_sceneImporter)
        {
            return unexpected<std::string>{"missing importer"};
        }
        auto result = (*_sceneImporter)(_sceneDef);
        if (result)
        {
            _app.getOrAddComponent<SceneAppComponent>().update(0.f);
        }
        return result;
    }

    void EditorProject::open()
    {
        if (!_scene)
        {
            return;
        }
        auto dialog = pfd::open_file("Open Project", ".",
            _dialogFilters);

        while (!dialog.ready(1000))
        {
            StreamUtils::logDebug("waiting for open dialog...");
        }
        if (dialog.result().empty())
        {
            return;
        }
        std::filesystem::path path = dialog.result()[0];
        if (!std::filesystem::exists(path))
        {
            return;
        }
        _path = path;

		auto result = protobuf::read(_sceneDef, _path);
        if (!result)
        {
            StreamUtils::logDebug("error reading scene: " + result.error());
            return;
        }
        updateScene();
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
        return _sceneImporter->getComponentLoadContext();
    }

    AssetPack& EditorProject::getAssets()
    {
		return _sceneImporter->getAssetPack();
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

        Camera::Definition cam;
        cam.set_perspective_fovy(glm::radians(60.f));
        cam.set_near(0.3f);
        cam.set_far(1000.f);

        //cam.addComponent<ShadowRenderer>(shadowConfig);
        //cam.addComponent<LightingRenderComponent>();
        //cam.addComponent<ForwardRenderer>();
        //cam.addComponent<FrustumCuller>();

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
        Renderable::Definition renderable;
        renderable.set_mesh_path(meshPath);
        renderable.set_material_path(matPath);
        scene.setComponent(renderableEntity, renderable);

        auto renderableTrans = Transform::createDefinition();
        renderableTrans.set_name("Sphere");
        scene.setComponent(renderableEntity, renderableTrans);
    }
}