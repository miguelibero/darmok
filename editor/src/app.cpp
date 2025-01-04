#include <darmok-editor/app.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok-editor/IconsMaterialDesign.h>

#include <darmok/window.hpp>
#include <darmok/transform.hpp>
#include <darmok/mesh_source.hpp>

#include <darmok/camera.hpp>
#include <darmok/light.hpp>
#include <darmok/render_scene.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <bx/commandline.h>
#include <CLI/CLI.hpp>

namespace darmok::editor
{
    EditorApp::EditorApp(App& app) noexcept
        : _app(app)
        , _sceneView(app)
        , _materialAssetsView("Materials", "MATERIAL")
        , _programAssetsView("Programs", "PROGRAM")
        , _meshAssetsView("Meshes", "MESH")
        , _proj(app)
        , _dockLeftId(0)
        , _dockRightId(0)
        , _dockCenterId(0)
        , _dockDownId(0)
        , _symbolsFont(nullptr)
        , _scenePlaying(false)
        , _mainToolbarHeight(0.F)
    {
    }

    EditorApp::~EditorApp() noexcept
    {
        // empty on purpose
    }    

    std::optional<int32_t> EditorApp::setup(const CmdArgs& args) noexcept
    {
        ReflectionUtils::bind();
        _inspectorView.setup();

        CLI::App cli{ "darmok editor" };

        auto& subCli = *cli.group("Program Compiler");
        subCli.add_option("--bgfx-shaderc", _progCompConfig.shadercPath, "path to the shaderc executable");
        std::vector<std::string> shaderIncludePaths;
        subCli.add_option("--bgfx-shader-include", shaderIncludePaths, "paths to shader files to be included");
            
        CLI11_PARSE(cli, args.size(), args.data());

        for (auto& path : shaderIncludePaths)
        {
            _progCompConfig.includePaths.emplace(path);
        }

        return std::nullopt;
    }

    void EditorApp::init()
    {
        auto& win = _app.getWindow();
        win.requestTitle("darmok editor");
        _app.setDebugFlag(BGFX_DEBUG_TEXT);

        _imgui = _app.getOrAddComponent<ImguiAppComponent>(*this);
        
        _proj.init(_progCompConfig);
        _sceneView.init(_proj.getScene(), _proj.getCamera().value());
        _inspectorView.init(*this);
        _materialAssetsView.init(*this);
        _programAssetsView.init(*this);
        _meshAssetsView.init(*this);
    }

    void EditorApp::shutdown()
    {
        stopScene();
        _inspectorView.shutdown();
        _materialAssetsView.shutdown();
        _programAssetsView.shutdown();
        _meshAssetsView.shutdown();
        _sceneView.shutdown();
        _proj.shutdown();
        _imgui.reset();
        _app.removeComponent<ImguiAppComponent>();
        _app.removeComponent<SceneAppComponent>();
        _dockDownId = 0;
        _dockRightId = 0;
        _dockLeftId = 0;
        _dockCenterId = 0;
        _symbolsFont = nullptr;
        _mainToolbarHeight = 0.F;
    }

    void EditorApp::imguiSetup()
    {
        ImGuiIO& io = ImGui::GetIO();

        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        io.Fonts->Clear();
        io.Fonts->AddFontFromFileTTF("assets/noto.ttf", 20);

        ImFontConfig config;
        config.GlyphMinAdvanceX = 30.0f; // Use if you want to make the icon monospaced
        static const ImWchar iconRanges[] = { ImWchar(ICON_MIN_MD), ImWchar(ICON_MAX_MD) , ImWchar(0) };
        _symbolsFont = io.Fonts->AddFontFromFileTTF("assets/MaterialIcons-Regular.ttf", 30.0f, &config, iconRanges);

        io.Fonts->Build();
    }

    const ImGuiWindowFlags EditorApp::_fixedFlags = ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    void EditorApp::renderDockspace()
    {
        ImGui::SetNextWindowPos(ImVec2(0, _mainToolbarHeight));
        auto size = ImGui::GetIO().DisplaySize;
        size.y -= _mainToolbarHeight;
        ImGui::SetNextWindowSize(size);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("DockSpace Window", nullptr, _fixedFlags);
        ImGui::PopStyleVar(2);

        // Create the dockspace
        auto dockId = ImGui::GetID("DockSpace");
        ImGui::DockSpace(dockId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
        ImGui::End();

        if (!_dockCenterId)
        {
            _dockRightId = ImGui::DockBuilderSplitNode(dockId, ImGuiDir_Right, 0.25f, nullptr, &dockId);
            _dockDownId = ImGui::DockBuilderSplitNode(dockId, ImGuiDir_Down, 0.25f, nullptr, &dockId);
            _dockLeftId = ImGui::DockBuilderSplitNode(dockId, ImGuiDir_Left, 0.25F, nullptr, &dockId);
            _dockCenterId = dockId;

            ImGui::DockBuilderDockWindow(_sceneTreeWindowName, _dockLeftId);
            ImGui::DockBuilderDockWindow(_inspectorView.getWindowName().c_str(), _dockRightId);
            ImGui::DockBuilderDockWindow(_sceneView.getWindowName().c_str(), _dockCenterId);
            ImGui::DockBuilderDockWindow(_materialAssetsView.getName(), _dockDownId);
            ImGui::DockBuilderDockWindow(_programAssetsView.getName(), _dockDownId);
            ImGui::DockBuilderDockWindow(_meshAssetsView.getName(), _dockDownId);
        }
    }   

    void EditorApp::playScene()
    {
        _scenePlaying = true;
    }

    void EditorApp::stopScene()
    {
        _scenePlaying = false;
    }

    void EditorApp::pauseScene()
    {
    }

    Entity EditorApp::getSelectedEntity() noexcept
    {
        auto scene = _proj.getScene();
        if (!scene)
        {
            return entt::null;
        }
        return _inspectorView.getSelectedEntity();
    }

    Entity EditorApp::addEntity() noexcept
    {
        auto scene = _proj.getScene();
        if (!scene)
        {
            return entt::null;
        }
        auto entity = scene->createEntity();
        auto& trans = scene->addComponent<Transform>(entity);
        trans.setName("New Entity");

        auto parentEntity = _inspectorView.getSelectedEntity();
        if (auto parentTrans = scene->getComponent<Transform>(parentEntity))
        {
            trans.setParent(parentTrans);
        }

        return entity;
    }

    void EditorApp::renderMainMenu()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open...", "Ctrl+O"))
                {
                    _proj.open();
                }
                if (ImGui::MenuItem("Save", "Ctrl+S"))
                {
                    _proj.save();
                }
                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
                {
                    _proj.save(true);
                }
                if (ImGui::MenuItem("Close", "Ctrl+W"))
                {
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit"))
                {
                    _app.quit();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Add Entity"))
                {
                    addEntity();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Component"))
            {
                auto disabled = getSelectedEntity() == entt::null;
                ImGui::BeginDisabled(disabled);
                if (ImGui::BeginMenu("Add"))
                {
                    drawEntityComponentMenu<Renderable>("Renderable");
                    drawEntityComponentMenu<Camera>("Camera");
                    if (ImGui::BeginMenu("Light"))
                    {
                        drawEntityComponentMenu<PointLight>("Point Light");
                        drawEntityComponentMenu<DirectionalLight>("Directional Light");
                        drawEntityComponentMenu<SpotLight>("Spot Light");
                        drawEntityComponentMenu<AmbientLight>("Ambient Light");
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndDisabled();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("About darmok"))
                {
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    void EditorApp::renderMainToolbar()
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
        ImGui::Begin("Main Toolbar", nullptr, _fixedFlags | ImGuiWindowFlags_MenuBar);
        ImGui::PopStyleVar();

        auto renderSeparator = []()
        {
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
            ImGui::SameLine();
        };

        ImGui::PushFont(_symbolsFont);

        {
            ImGui::BeginGroup();
            if (ImGui::ButtonEx(ICON_MD_SAVE))
            {
                _proj.save();
            }
            ImGui::SameLine();
            if (ImGui::ButtonEx(ICON_MD_FOLDER_OPEN))
            {
                _proj.open();
            }
            ImGui::EndGroup();
            ImGui::SameLine();
        }

        renderSeparator();

        {
            ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_SingleSelect;
            auto ms = ImGui::BeginMultiSelect(flags);

            auto renderOption = [this](const char* label, TransformGizmoMode mode)
            {
                auto selected = _sceneView.getTransformGizmoMode() == mode;
                auto size = ImGui::CalcTextSize(label);
                if (ImGui::Selectable(label, selected, 0, size))
                {
                    _sceneView.setTransformGizmoMode(mode);
                }
                ImGui::SameLine();
            };

            renderOption(ICON_MD_BACK_HAND, TransformGizmoMode::Grab);
            renderOption(ICON_MD_OPEN_WITH, TransformGizmoMode::Translate);
            renderOption(ICON_MD_3D_ROTATION, TransformGizmoMode::Rotate);
            renderOption(ICON_MD_SCALE, TransformGizmoMode::Scale);

            ImGui::EndMultiSelect();
            ImGui::SameLine();
        }

        renderSeparator();

        {
            ImGui::BeginGroup();
            if (_scenePlaying)
            {
                if (ImGui::Button(ICON_MD_STOP))
                {
                    stopScene();
                }
            }
            else
            {
                if (ImGui::Button(ICON_MD_PLAY_ARROW))
                {
                    playScene();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_MD_PAUSE))
            {
                pauseScene();
            }
            ImGui::EndGroup();
            ImGui::SameLine();
        }

        ImGui::PopFont();
        _mainToolbarHeight = ImGui::GetWindowSize().y;
        ImGui::End();
    }

    void EditorApp::onSceneTreeSceneClicked()
    {
        onObjectSelected(Entity(entt::null));
    }

    void EditorApp::onObjectSelected(const SelectableObject& obj) noexcept
    {
        _inspectorView.selectObject(obj, _proj.getScene());
        auto entity = _inspectorView.getSelectedEntity();
        _sceneView.selectEntity(entity);
    }

    void EditorApp::onSceneTreeTransformClicked(Transform& trans)
    {
        if (auto scene = _proj.getScene())
        {
            onObjectSelected(scene->getEntity(trans));
        }
    }

    const char* EditorApp::_sceneTreeWindowName = "Scene Tree";

    void EditorApp::renderSceneTree()
    {
        auto scene = _proj.getScene();
        if (!scene)
        {
            return;
        }
        if (ImGui::Begin(_sceneTreeWindowName))
        {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
            auto selectedScene = _inspectorView.getSelectedScene();
            if (selectedScene == scene)
            {
                flags |= ImGuiTreeNodeFlags_Selected;
            }
            auto sceneName = _proj.getScene()->getName();
            if (sceneName.empty())
            {
                sceneName = "Scene";
            }
            if (ImGui::TreeNodeEx(sceneName.c_str(), flags))
            {
                if (ImGui::IsItemClicked())
                {
                    onSceneTreeSceneClicked();
                }
                for (auto& entity : scene->getRootEntities())
                {
                    auto& trans = scene->getComponent<Transform>(entity).value();
                    renderSceneTreeBranch(trans);
                }
                ImGui::TreePop();
            }
            if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete))
            {
                auto entity = _inspectorView.getSelectedEntity();
                scene->destroyEntityImmediate(entity, true);
                onObjectSelected(Entity(entt::null));
            }
        }
        ImGui::End();
    }

    void EditorApp::renderSceneTreeBranch(Transform& trans)
    {
        std::string name = trans.getName();
        if (name.empty())
        {
            name = "Entity";
        }
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
        auto children = trans.getChildren();
        if (children.empty())
        {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }
        auto selectedEntity = _inspectorView.getSelectedEntity();
        auto entity = _proj.getScene()->getEntity(trans);
        if (selectedEntity == entity)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        if (ImGui::TreeNodeEx(name.c_str(), flags))
        {
            if (ImGui::IsItemClicked())
            {
                onSceneTreeTransformClicked(trans);
            }
            for (auto& child : children)
            {
                renderSceneTreeBranch(child.get());
            }
            ImGui::TreePop();
        }
    }

    void EditorApp::imguiRender()
    {
        _sceneView.beforeRender();
        renderMainMenu();
        renderDockspace();
        renderMainToolbar();
        renderSceneTree();
        _inspectorView.render();
        _sceneView.render();
        _materialAssetsView.render();
        _programAssetsView.render();
        _meshAssetsView.render();
    }

    void EditorApp::update(float deltaTime)
    {
        _sceneView.update(deltaTime);
    }

    EditorProject& EditorApp::getProject() noexcept
    {
        return _proj;
    }

    const EditorProject& EditorApp::getProject() const noexcept
    {
        return _proj;
    }

    AssetContext& EditorApp::getAssets() noexcept
    {
        return _app.getAssets();
    }

    const AssetContext& EditorApp::getAssets() const noexcept
    {
        return _app.getAssets();
    }

    bool EditorApp::drawMaterialReference(const char* label, std::shared_ptr<Material>& mat)
    {
        auto name = _proj.getMaterialName(mat);
        auto action = ImguiUtils::drawAssetReference(label, mat, name, "MATERIAL");
        if (action == ReferenceInputAction::Changed)
        {
            return true;
        }
        if (action == ReferenceInputAction::Visit)
        {
            onAssetSelected(mat);
        }
        return false;
    }

    bool EditorApp::drawProgramReference(const char* label, std::shared_ptr<Program>& prog)
    {
        auto name = _proj.getProgramName(prog);
        auto asset = _proj.findProgram(prog);
        auto action = ImguiUtils::drawAssetReference(label, asset, name, "PROGRAM");
        if (action == ReferenceInputAction::Changed)
        {
            // TODO: show progress when compiling program
            prog = _proj.loadProgram(asset);
            return true;
        }
        if (action == ReferenceInputAction::Visit)
        {
            onAssetSelected(asset);
        }
        return false;
    }

    bool EditorApp::drawMeshReference(const char* label, std::shared_ptr<IMesh>& mesh, const bgfx::VertexLayout& layout)
    {
        auto name = _proj.getMeshName(mesh);
        auto asset = _proj.findMesh(mesh);
        auto action = ImguiUtils::drawAssetReference(label, asset, name, "MESH");
        if (action == ReferenceInputAction::Changed)
        {
            // TODO: show progress when loading mesh
            mesh = _proj.loadMesh(asset, layout);
            return true;
        }
        if (action == ReferenceInputAction::Visit)
        {
            onAssetSelected(asset);
        }
        return false;
    }

    std::vector<MaterialAsset> EditorApp::getAssets(std::type_identity<MaterialAsset>) const
    {
        return _proj.getMaterials();
    }

    std::optional<MaterialAsset> EditorApp::getSelectedAsset(std::type_identity<MaterialAsset>) const
    {
        return _inspectorView.getSelectedObject<MaterialAsset>();
    }

    void EditorApp::onAssetSelected(const MaterialAsset& asset)
    {
        onObjectSelected(asset);
        _materialAssetsView.focus();
    }

    std::string EditorApp::getAssetName(const MaterialAsset& asset) const
    {
        return asset->getName();
    }

    void EditorApp::addAsset(std::type_identity<MaterialAsset>)
    {
        _proj.addMaterial();
    }

    std::vector<ProgramAsset> EditorApp::getAssets(std::type_identity<ProgramAsset>) const
    {
        return _proj.getPrograms();
    }

    std::optional<ProgramAsset> EditorApp::getSelectedAsset(std::type_identity<ProgramAsset>) const
    {
        return _inspectorView.getSelectedObject<ProgramAsset>();
    }

    void EditorApp::onAssetSelected(const ProgramAsset& asset)
    {
        onObjectSelected(asset);
        _programAssetsView.focus();
    }

    std::string EditorApp::getAssetName(const ProgramAsset& asset) const
    {
        if (auto standard = std::get_if<StandardProgramType>(&asset))
        {
            return StandardProgramLoader::getTypeName(*standard);
        }
        auto ptr = std::get<std::shared_ptr<ProgramSource>>(asset);
        return ptr ? ptr->name : "";
    }

    void EditorApp::addAsset(std::type_identity<ProgramAsset>) 
    {
        _proj.addProgram();
    }

    std::vector<MeshAsset> EditorApp::getAssets(std::type_identity<MeshAsset>) const
    {
        return _proj.getMeshes();
    }

    std::optional<MeshAsset> EditorApp::getSelectedAsset(std::type_identity<MeshAsset>) const
    {
        return _inspectorView.getSelectedObject<MeshAsset>();
    }

    std::string EditorApp::getAssetName(const MeshAsset& asset) const
    {
        return asset->name;
    }

    void EditorApp::onAssetSelected(const MeshAsset& asset)
    {
        onObjectSelected(asset);
        _meshAssetsView.focus();
    }

    void EditorApp::addAsset(std::type_identity<MeshAsset>)
    {
        _proj.addMesh();
    }
}