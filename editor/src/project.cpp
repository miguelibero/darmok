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

    void EditorProject::init(const ProgramCompilerConfig& progCompilerConfig)
    {
        doReset();
        _progCompiler.emplace(progCompilerConfig);
        _requestReset = false;
        _requestUpdateScene = false;
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

    void EditorProject::render()
    {
        if (_requestReset)
        {
            _requestReset = false;
            ImGui::OpenPopup(_confirmNewPopup);
        }
        auto action = ImguiUtils::drawConfirmPopup(_confirmNewPopup, "Are you sure you want to create a new project?");
        if (action == ConfirmPopupAction::Ok)
        {
            doReset();
        }
        if (_requestUpdateScene)
        {
            doUpdateScene();
        }
    }

    void EditorProject::updateScene()
    {
        _requestUpdateScene = true;
    }

    const char* EditorProject::_confirmNewPopup = "Confirm New Project";

    void EditorProject::reset()
    {
        _requestReset = true;
    }

    void EditorProject::doReset()
    {
        _requestReset = false;
        _path.clear();

        auto& scenes = _app.getOrAddComponent<SceneAppComponent>();
        _scene = scenes.getScene();
		_sceneImporter.emplace(*_scene, _assetPackConfig);

        _scene->destroyEntitiesImmediate();
        configureDefaultScene(_sceneWrapper);
        configureEditorScene(*_scene);
        doUpdateScene();
    }

    void EditorProject::doUpdateScene()
    {
        _requestUpdateScene = false;
        if (_sceneImporter)
        {
            (*_sceneImporter)(_sceneDef);
            _app.getOrAddComponent<SceneAppComponent>().update(0.f);
        }
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
        }
    }

    std::shared_ptr<Scene> EditorProject::getScene()
    {
        return _scene;
    }

    SceneDefinitionWrapper& EditorProject::getSceneDefinition()
    {
        return _sceneWrapper;
    }

    OptionalRef<Camera> EditorProject::getCamera()
    {
        return _cam;
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

        auto scale = protobuf::convert(glm::vec3{ 1.f });
        auto white = protobuf::convert(Colors::white());
        auto white3 = protobuf::convert(Colors::white3());

        Transform::Definition camTrans;
		*camTrans.mutable_scale() = scale;
        camTrans.set_name("Main Camera");
        *camTrans.mutable_position() = protobuf::convert(glm::vec3{ 0.f, 1.f, -10.f });
        scene.setComponent(camEntity, camTrans);

        auto lightsEntity = scene.createEntity();
        Transform::Definition lightsTrans;
        lightsTrans.set_name("Lighting");
        scene.setComponent(lightsEntity, lightsTrans);

        auto ambLightEntity = scene.createEntity();
        AmbientLight::Definition ambLight;
        ambLight.set_intensity(0.2f);
        *ambLight.mutable_color() = white3;
        scene.setComponent(ambLightEntity, ambLight);
        Transform::Definition ambLightTrans;
        ambLightTrans.set_name("Ambient Light");
        ambLightTrans.set_parent(entt::to_integral(lightsEntity));
        *ambLightTrans.mutable_scale() = scale;
        scene.setComponent(ambLightEntity, ambLightTrans);

        auto dirLightEntity = scene.createEntity();
        DirectionalLight::Definition dirLight;
        dirLight.set_intensity(3.f);
        *dirLight.mutable_color() = white3;
        scene.setComponent(dirLightEntity, dirLight);
        Transform::Definition dirLightTrans;
        dirLightTrans.set_name("Directional Light");
        dirLightTrans.set_parent(entt::to_integral(lightsEntity));
        *dirLightTrans.mutable_scale() = scale;
        *dirLightTrans.mutable_position() = protobuf::convert(glm::vec3{ 0.f, 3.f, 0.f });
        auto rot = glm::quat{ glm::radians(glm::vec3{50.f, -30.f, 0.f}) };
        *dirLightTrans.mutable_rotation() = protobuf::convert(rot);
        scene.setComponent(dirLightEntity, dirLightTrans);

        Mesh::Source mesh;
		mesh.set_standard_program(Program::Standard::Forward);
        mesh.set_name("Default Shape");
		auto& sphere = *mesh.mutable_sphere();
        sphere.mutable_shape()->set_radius(1.f);
        sphere.set_lod(32);
        std::string meshPath = "default_mesh";
        scene.setAsset(meshPath, mesh);

        Material::Definition mat;
        mat.set_name("Default");
        mat.set_standard_program(Program::Standard::Forward);
        *mat.mutable_base_color() = white;
        mat.set_opacity_type(Material::Definition::Opaque);
        std::string matPath = "default_material";
        scene.setAsset(matPath, mat);

        auto renderableEntity = scene.createEntity();
        Renderable::Definition renderable;
        renderable.set_mesh_path(meshPath);
        renderable.set_material_path(matPath);
        scene.setComponent(renderableEntity, renderable);

        Transform::Definition renderableTrans;
        renderableTrans.set_name("Sphere");
        *renderableTrans.mutable_scale() = scale;
        scene.setComponent(renderableEntity, renderableTrans);
    }
}