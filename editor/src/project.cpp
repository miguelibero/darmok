#include <darmok-editor/project.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include <darmok/texture.hpp>
#include <darmok/camera.hpp>
#include <darmok/stream.hpp>
#include <darmok/material.hpp>
#include <darmok/program.hpp>

#include <portable-file-dialogs.h>

// default scene
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
    {
    }

    void EditorProject::init()
    {
        auto& scenes = _app.getOrAddComponent<SceneAppComponent>();
        _scene = scenes.getScene();
        _scenes.push_back(_scene);

        configureEditorScene(*_scene);
        configureDefaultScene(*_scene);
    }

    void EditorProject::shutdown()
    {
        _path.reset();
    }

    void EditorProject::save(bool forceNewPath)
    {
        if (!_scene)
        {
            return;
        }
        if (!_path || forceNewPath)
        {
            std::string initialPath(".");
            if (_path && std::filesystem::exists(_path.value()))
            {
                initialPath = _path.value().string();
            }
            auto dialog = pfd::save_file("Save Project", initialPath,
                _dialogFilters, pfd::opt::force_path);

            while (!dialog.ready(1000))
            {
                StreamUtils::logDebug("waiting for save dialog...");
            }
            _path = dialog.result();
        }
        if (_path)
        {
            CerealUtils::save(*this, _path.value());
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
        std::filesystem::path path = dialog.result()[0];
        if (!std::filesystem::exists(path))
        {
            return;
        }
        CerealUtils::load(*this, path);
        _path = path;
    }

    std::shared_ptr<Scene> EditorProject::getScene()
    {
        return _scene;
    }

    OptionalRef<Camera> EditorProject::getCamera()
    {
        return _cam;
    }

    EditorProject::Materials& EditorProject::getMaterials()
    {
        return _materials;
    }

    const EditorProject::Materials& EditorProject::getMaterials() const
    {
        return _materials;
    }

    EditorProject::Programs& EditorProject::getPrograms()
    {
        return _programs;
    }

    const EditorProject::Programs& EditorProject::getPrograms() const
    {
        return _programs;
    }

    bool EditorProject::shouldCameraRender(const Camera& cam) const noexcept
    {
        return _cam.ptr() == &cam;
    }

    bool EditorProject::shouldEntityBeSerialized(Entity entity) const noexcept
    {
        return !isEditorEntity(entity);
    }

    bool EditorProject::isEditorEntity(Entity entity) const noexcept
    {
        if (_scene && _cam && _scene->getEntity(_cam.value()) == entity)
        {
            return true;
        }
        return false;
    }

    void EditorProject::configureEditorScene(Scene& scene)
    {
        scene.setDelegate(*this);

        static const std::string name = "Editor Camera";
        auto skyboxTex = _app.getAssets().getTextureLoader()("cubemap.ktx");
        auto camEntity = scene.createEntity();
        auto& cam = scene.addComponent<Camera>(camEntity);

        scene.addComponent<Transform>(camEntity)
            .setPosition(glm::vec3(10, 5, -10))
            .lookAt(glm::vec3(0))
            .setName(name);
        cam.setPerspective(60.F, 0.3F, 10000.F);
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

    void EditorProject::configureDefaultScene(Scene& scene)
    {
        ShadowRendererConfig shadowConfig;
        shadowConfig.cascadeAmount = 3;

        auto camEntity = scene.createEntity();
        auto& cam = scene.addComponent<Camera>(camEntity)
            .setPerspective(60.F, 0.3F, 1000.F);
        cam.addComponent<ShadowRenderer>(shadowConfig);
        cam.addComponent<LightingRenderComponent>();
        cam.addComponent<ForwardRenderer>();
        cam.addComponent<FrustumCuller>();

        scene.addComponent<Transform>(camEntity, glm::vec3(0.F, 1.F, -10.F))
            .setName("Main Camera");
        auto lightEntity = scene.createEntity();
        auto& light = scene.addComponent<DirectionalLight>(lightEntity);
        scene.addComponent<Transform>(lightEntity, glm::vec3(0.F, 3.F, 0.F))
            .setEulerAngles(glm::vec3(50.F, -30.F, 0.F))
            .setName("Directional Light");

        auto cubeEntity = scene.createEntity();

        auto& assets = _app.getAssets();

        auto prog = StandardProgramLoader::load(StandardProgramType::Forward);

        auto meshDef = std::make_shared<MeshDefinition>(
            MeshData(Cube()).createMeshDefinition(prog->getVertexLayout())
        );
        auto mesh = assets.getMeshLoader().loadResource(meshDef);

        auto mat = std::make_shared<Material>(prog);
        mat->setName("Default");
        mat->setBaseColor(Colors::white());
        _materials.push_back(mat);

        scene.addComponent<Renderable>(cubeEntity, mesh, mat);
        scene.addComponent<Transform>(cubeEntity)
            .setName("Cube");
    }
}