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
        : _app(app)
        , _tryReset(false)
        , _sceneWrapper{ _sceneDef }
    {
    }

    void EditorProject::init(const ProgramCompilerConfig& progCompilerConfig)
    {
        doReset();
        _progCompiler.emplace(progCompilerConfig);
        _tryReset = false;
    }

    void EditorProject::shutdown()
    {
        _path = "";
        _tryReset = false;
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
    }

    void EditorProject::exportScene()
    {

    }

    void EditorProject::render()
    {
        if (_tryReset)
        {
            _tryReset = false;
            ImGui::OpenPopup(_confirmNewPopup);
        }

        auto action = ImguiUtils::drawConfirmPopup(_confirmNewPopup, "Are you sure you want to create a new project?");
        if (action == ConfirmPopupAction::Ok)
        {
            doReset();
        }
    }

    const char* EditorProject::_confirmNewPopup = "Confirm New Project";

    void EditorProject::reset()
    {
        _tryReset = true;
    }

    void EditorProject::doReset()
    {
        _path.clear();

        auto& scenes = _app.getOrAddComponent<SceneAppComponent>();
        _scene = scenes.getScene();

        _scene->destroyEntitiesImmediate();
        configureEditorScene(*_scene);
        configureDefaultScene(_sceneDef);
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
        auto skyboxTex = _app.getAssets().getTextureLoader()("cubemap.ktx");
        auto camEntity = scene.createEntity();
        auto& cam = scene.addComponent<Camera>(camEntity);

        scene.addComponent<Transform>(camEntity)
            .setPosition(glm::vec3(2, 1, -2))
            .lookAt(glm::vec3(0))
            .setName(name);
        cam.setPerspective(60.F, 0.3F, 1000.F);
        cam.addComponent<SkyboxRenderer>(skyboxTex);
        cam.addComponent<GridRenderer>();
        cam.addComponent<LightingRenderComponent>();
        ShadowRendererConfig shadowConfig;
        shadowConfig.cascadeAmount = 3;
        cam.addComponent<ShadowRenderer>(shadowConfig);
        cam.addComponent<ForwardRenderer>();
        cam.addComponent<FrustumCuller>();
        _cam = cam;
    }

    void EditorProject::configureDefaultScene(SceneDefinition& scene)
    {
        scene.set_name("Scene");

        ShadowRendererConfig shadowConfig;
        shadowConfig.cascadeAmount = 3;

        auto camEntity = SceneDefinitionUtils::createEntity(scene);

        Camera::Definition cam;
        cam.set_perspective_fovy(glm::radians(60.f));
        cam.set_near(0.3f);
        cam.set_far(1000.f);

        //cam.addComponent<ShadowRenderer>(shadowConfig);
        //cam.addComponent<LightingRenderComponent>();
        //cam.addComponent<ForwardRenderer>();
        //cam.addComponent<FrustumCuller>();

        SceneDefinitionUtils::addComponent(scene, camEntity, cam);

        Transform::Definition camTrans;
        camTrans.set_name("Main Camera");
        *camTrans.mutable_position() = protobuf::convert(glm::vec3{ 0.f, 1.f, -10.f });
        SceneDefinitionUtils::addComponent(scene, camEntity, camTrans);

        auto ambLightEntity = SceneDefinitionUtils::createEntity(scene);
        AmbientLight::Definition ambLight;
        ambLight.set_intensity(0.2f);
        SceneDefinitionUtils::addComponent(scene, ambLightEntity, ambLight);
        Transform::Definition ambLightTrans;
        ambLightTrans.set_name("Ambient Light");
        SceneDefinitionUtils::addComponent(scene, ambLightEntity, ambLightTrans);

        auto dirLightEntity = SceneDefinitionUtils::createEntity(scene);
        DirectionalLight::Definition dirLight;
        dirLight.set_intensity(3.f);
        SceneDefinitionUtils::addComponent(scene, dirLightEntity, dirLight);
        Transform::Definition dirLightTrans;
        ambLightTrans.set_name("Directional Light");
        *ambLightTrans.mutable_position() = protobuf::convert(glm::vec3{ 0.f, 3.f, 0.f });
        auto rot = glm::quat{ glm::radians(glm::vec3{50.f, -30.f, 0.f}) };
        *ambLightTrans.mutable_rotation() = protobuf::convert(rot);
        SceneDefinitionUtils::addComponent(scene, dirLightEntity, dirLightTrans);

        Mesh::Source mesh;
        mesh.set_name("Default Shape");
        mesh.mutable_sphere();
        std::string meshPath = "default_mesh";
        SceneDefinitionUtils::addAsset(scene, meshPath, mesh);

        Material::Definition mat;
        mat.set_name("Default");
        mat.set_standard_program(Program::Standard::Forward);
        *mat.mutable_base_color() = protobuf::convert(Colors::white());
        mat.set_opacity_type(Material::Definition::Opaque);
        std::string matPath = "default_material";
        SceneDefinitionUtils::addAsset(scene, matPath, mat);

        auto renderableEntity = SceneDefinitionUtils::createEntity(scene);
        Renderable::Definition renderable;
        renderable.set_mesh_path(meshPath);
        renderable.set_material_path(matPath);
        SceneDefinitionUtils::addComponent(scene, renderableEntity, renderable);

        Transform::Definition renderableTrans;
        renderableTrans.set_name("Sphere");
        SceneDefinitionUtils::addComponent(scene, renderableEntity, renderableTrans);
    }
}