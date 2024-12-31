#include <darmok-editor/project.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include <darmok/texture.hpp>
#include <darmok/camera.hpp>
#include <darmok/stream.hpp>
#include <darmok/material.hpp>
#include <darmok/program.hpp>
#include <darmok/mesh_shape.hpp>

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

    void EditorProject::init(const ProgramCompilerConfig& progCompilerConfig)
    {
        auto& scenes = _app.getOrAddComponent<SceneAppComponent>();
        _scene = scenes.getScene();
        _scenes.insert(_scene);

        configureEditorScene(*_scene);
        configureDefaultScene(*_scene);

        _progCompiler.emplace(progCompilerConfig);
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

    const EditorProject::Materials& EditorProject::getMaterials() const
    {
        return _materials;
    }

    std::shared_ptr<Material> EditorProject::addMaterial()
    {
        auto mat = *_materials.emplace().first;
        mat->setName("New Material");
        return mat;
    }

    const EditorProject::Programs& EditorProject::getPrograms() const
    {
        return _programs;
    }

    bool EditorProject::removeProgram(ProgramSource& src) noexcept
    {
        auto ptr = &src;
        auto itr = std::find_if(_programs.begin(), _programs.end(),
            [ptr](auto& elm) { return elm.first.get() == ptr; });
        if (itr != _programs.end())
        {
            _programs.erase(itr);
            return true;
        }
        return false;
    }

    std::string EditorProject::getProgramName(const std::shared_ptr<Program>& prog) const
    {
        auto def = _app.getAssets().getProgramLoader().getDefinition(prog);
        std::string name;
        if (def)
        {
            return def->name;
        }
        if (auto standard = StandardProgramLoader::getType(prog))
        {
            return StandardProgramLoader::getTypeName(standard.value());
        }
        return "Unnamed Program";
    }

    bool EditorProject::isProgramCached(const ProgramAsset& asset) const noexcept
    {
        if (std::holds_alternative<StandardProgramType>(asset))
        {
            return true;
        }
        auto src = std::get<std::shared_ptr<ProgramSource>>(asset);
        auto itr = _programs.find(src);
        return itr != _programs.end() && itr->second != nullptr;
    }

    std::shared_ptr<ProgramSource> EditorProject::addProgram()
    {
        auto src = std::make_shared<ProgramSource>();
        src->name = "New Program";
        _programs[src] = nullptr;
        return src;
    }

    std::shared_ptr<Program> EditorProject::loadProgram(const ProgramAsset& asset)
    {
        if (auto standard = std::get_if<StandardProgramType>(&asset))
        {
            return StandardProgramLoader::load(*standard);
        }
        auto src = std::get<std::shared_ptr<ProgramSource>>(asset);
        auto itr = _programs.find(src);
        std::shared_ptr<ProgramDefinition> def;
        if (itr != _programs.end())
        {
            def = itr->second;
        }
        if(def == nullptr)
        {
            if (!_progCompiler)
            {
                return nullptr;
            }
            def = std::make_shared<ProgramDefinition>((*_progCompiler)(*src));
        }
        auto& loader = _app.getAssets().getProgramLoader();
        return loader.loadResource(def);
    }

    const EditorProject::Meshes& EditorProject::getMeshes() const
    {
        return _meshes;
    }

    std::string EditorProject::getMeshName(const std::shared_ptr<IMesh>& mesh) const
    {
        auto& loader = _app.getAssets().getMeshLoader();
        auto def = loader.getDefinition(mesh);
        return def ? def->name : "";
    }

    std::shared_ptr<IMesh> EditorProject::loadMesh(const MeshAsset& asset)
    {
        return nullptr;
    }

    std::shared_ptr<MeshSource> EditorProject::addMesh()
    {
        auto src = std::make_shared<MeshSource>();
        src->name = "New Mesh";
        src->shape.emplace<SphereMeshSource>();
        _meshes[src] = nullptr;
        return src;
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
        _materials.insert(mat);

        scene.addComponent<Renderable>(cubeEntity, mesh, mat);
        scene.addComponent<Transform>(cubeEntity)
            .setName("Cube");
    }
}