#include <darmok-editor/app.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok-editor/IconsMaterialDesign.h>

#include <darmok/window.hpp>
#include <darmok/transform.hpp>

#include <darmok/camera.hpp>
#include <darmok/light.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/scene_serialize.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <CLI/CLI.hpp>

namespace darmok::editor
{
    EditorApp::EditorApp(App& app) noexcept
        : _app{ app }
        , _sceneView{ app }
        , _proj{app}
        , _dockLeftId{0}
        , _dockRightId{0}
        , _dockCenterId{0}
        , _dockDownId{0}
        , _symbolsFont{nullptr}
        , _scenePlaying{false}
        , _mainToolbarHeight{0.f}
    {
    }

    EditorApp::~EditorApp() noexcept
    {
        // empty on purpose
    }    

    std::optional<int32_t> EditorApp::setup(const CmdArgs& args) noexcept
    {
        _inspectorView.setup();

        CLI::App cli{ "darmok editor" };
        std::string version = "VERSION " DARMOK_VERSION;
        cli.set_version_flag("-v,--version", version);

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
        
        auto projResult = _proj.init(_progCompConfig);
        if(!projResult)
        {
            throw std::runtime_error("failed to initialize project: " + projResult.error());
		}
        _sceneView.init(_proj.getScene(), _proj.getCamera().value());
        _inspectorView.init(*this);
        _assetsView.init(_proj.getSceneDefinition(), *this);
    }

    void EditorApp::shutdown()
    {
        stopScene();
        _inspectorView.shutdown();
        _assetsView.shutdown();
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
        _fileInputResults.clear();
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
        auto& io = ImGui::GetIO();
        auto size = io.DisplaySize;
        size.x /= io.DisplayFramebufferScale.x;
        size.y /= io.DisplayFramebufferScale.y;
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
            ImGui::DockBuilderDockWindow(_assetsView.getWindowName().c_str(), _dockDownId);
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
        return _inspectorView.getSelectedEntity();
    }

    void EditorApp::renderMainMenu()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New", "Ctrl+N"))
                {
                    _proj.resetScene();
                }
                if (ImGui::MenuItem("Open...", "Ctrl+O"))
                {
                    _proj.openScene();
                }
                if (ImGui::MenuItem("Save", "Ctrl+S"))
                {
                    _proj.saveScene();
                }
                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
                {
                    _proj.saveScene(true);
                }
                if (ImGui::MenuItem("Export..."))
                {
                    _proj.exportScene();
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
                    _proj.addEntity(_inspectorView.getSelectedEntity());
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Asset"))
            {
                if (ImGui::BeginMenu("Add"))
                {
                    drawAssetComponentMenu("Program", Program::createSource());
                    drawAssetComponentMenu("Texture", Texture::createSource());
                    drawAssetComponentMenu("Mesh", Mesh::createSource());
                    drawAssetComponentMenu("Material", Material::createDefinition());
                    if (ImGui::BeginMenu("Animation"))
                    {
                        drawAssetComponentMenu("Armature", Armature::createDefinition());
                        drawAssetComponentMenu("Animator", SkeletalAnimator::createDefinition());
                        ImGui::EndMenu();
                    }
					ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Component"))
            {
                auto disabled = getSelectedEntity() == entt::null;
                ImGui::BeginDisabled(disabled);
                if (ImGui::BeginMenu("Add"))
                {
                    drawEntityComponentMenu("Renderable", Renderable::createDefinition());
                    drawEntityComponentMenu("Camera", Camera::createDefinition());
                    if (ImGui::BeginMenu("Light"))
                    {
                        drawEntityComponentMenu("Point Light", PointLight::createDefinition());
                        drawEntityComponentMenu("Directional Light", DirectionalLight::createDefinition());
                        drawEntityComponentMenu("Spot Light", SpotLight::createDefinition());
                        drawEntityComponentMenu("Ambient Light", AmbientLight::createDefinition());
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Animation"))
                    {
                        drawEntityComponentMenu("Skinnable", Skinnable::createDefinition());
                        drawEntityComponentMenu("Animator", SkeletalAnimator::createDefinition());
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
                    renderAboutDialog();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    void EditorApp::drawAssetComponentMenu(const char* name, const google::protobuf::Message& asset) noexcept
    {
        if (ImGui::MenuItem(name))
        {
            _assetsView.addAsset(asset);
        }
    }

    void EditorApp::drawEntityComponentMenu(const char* name, const google::protobuf::Message& comp) noexcept
    {
        auto& scene = _proj.getSceneDefinition();
        auto disabled = true;
        auto entity = _inspectorView.getSelectedEntity();
        if (entity != entt::null && !scene.getComponent(entity, protobuf::getTypeId(comp)))
        {
            disabled = false;
        }
        ImGui::BeginDisabled(disabled);
        if (ImGui::MenuItem(name))
        {
            scene.setComponent(entity, comp);
        }
        ImGui::EndDisabled();
    }

    void EditorApp::renderAboutDialog()
    {
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
                _proj.saveScene();
            }
            ImGui::SameLine();
            if (ImGui::ButtonEx(ICON_MD_FOLDER_OPEN))
            {
                _proj.openScene();
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
        onObjectSelected(Entity{});
    }

    void EditorApp::onObjectSelected(const SelectableObject& obj) noexcept
    {
        _inspectorView.selectObject(obj);
        auto entity = _inspectorView.getSelectedEntity();
        _sceneView.selectEntity(entity);
    }

    void EditorApp::onSceneTreeTransformClicked(Entity entity)
    {
        onObjectSelected(entity);
    }

    const char* EditorApp::_sceneTreeWindowName = "Scene Tree";

    bool EditorApp::renderSceneTree()
    {
        auto& scene = _proj.getSceneDefinition();
        auto changed = false;
        if (ImGui::Begin(_sceneTreeWindowName))
        {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
            if (_inspectorView.isSceneSelected())
            {
                flags |= ImGuiTreeNodeFlags_Selected;
            }
            std::string sceneName = scene.getName();
            if (sceneName.empty())
            {
                sceneName = "Scene";
            }
            if (ImGui::TreeNodeEx(sceneName.c_str(), flags))
            {
                if (renderEntityDragDropTarget(Entity{ 0 }))
                {
                    changed = true;
                }
                if (ImGui::IsItemClicked())
                {
                    onSceneTreeSceneClicked();
                }
                for (auto& entity : scene.getRootEntities())
                {
                    if (renderSceneTreeBranch(entity))
                    {
                        changed = true;
                    }
                }
                ImGui::TreePop();
            }
            if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete))
            {
                auto entity = _inspectorView.getSelectedEntity();
                if (scene.destroyEntity(entity))
                {
                    onObjectSelected(Entity{});
                    changed = true;
                }
            }
        }
        ImGui::End();
        return changed;
    }

    bool EditorApp::renderSceneTreeBranch(Entity entity)
    {
        auto& scene = _proj.getSceneDefinition();
        std::string name;
        if (auto trans = scene.getComponent<Transform::Definition>(entity))
        {
            name = trans->name();
        }
        if (name.empty())
        {
            name = "Entity";
        }
        
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
        auto children = scene.getChildren(entity);
        if (children.empty())
        {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }
        if (_inspectorView.getSelectedEntity() == entity)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        auto treeExpanded = ImGui::TreeNodeEx(name.c_str(), flags);
        auto changed = false;

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::TextUnformatted(name.c_str());
            ImGui::SetDragDropPayload(entityDragType, &entity, sizeof(Entity));
            ImGui::EndDragDropSource();
        }
        else
        {
            if (renderEntityDragDropTarget(entity))
            {
                changed = true;
            }
        }
        if (treeExpanded)
        {
            if (ImGui::IsItemClicked())
            {
                onSceneTreeTransformClicked(entity);
            }
            for (auto& child : children)
            {
                if (renderSceneTreeBranch(child))
                {
                    changed = true;
                }
            }
            ImGui::TreePop();
        }

        return changed;
    }

    bool EditorApp::renderEntityDragDropTarget(Entity entity)
    {
        if (!ImGui::BeginDragDropTarget())
        {
            return false;
        }
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(entityDragType);
        bool changed = false;
        if (payload)
        {
            IM_ASSERT(payload->DataSize == sizeof(Entity));
            auto droppedEntity = *static_cast<Entity*>(payload->Data);
            if (droppedEntity != entity)
            {
                auto& scene = _proj.getSceneDefinition();
                if (auto trans = scene.getComponent<Transform::Definition>(droppedEntity))
                {
                    auto entityId = entt::to_integral(entity);
                    if (trans->parent() != entityId)
                    {
                        trans->set_parent(entityId);
                        scene.setComponent(droppedEntity, *trans);
                        changed = true;
                    }
                }
            }
        }
        ImGui::EndDragDropTarget();
        return changed;
    }

    void EditorApp::imguiRender()
    {
        auto changed = false;
        _sceneView.beforeRender();
        renderMainMenu();
        renderDockspace();
        renderMainToolbar();
        if (renderSceneTree())
        {
            changed = true;
        }
        if (_inspectorView.render())
        {
            changed = true;
        }
        /*
        if (changed)
        {
            _proj.updateScene();
        }
        */

        auto projResult = _proj.render();
        if (!projResult)
        {
            throw std::runtime_error("failed to render project: " + projResult.error());
        }
        _sceneView.render();
        _assetsView.render();
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

    const Window& EditorApp::getWindow() const noexcept
    {
        return _app.getWindow();
    }

    Window& EditorApp::getWindow() noexcept
    {
        return _app.getWindow();
    }

    bool EditorApp::drawFileInput(const char* label, std::filesystem::path& path, FileDialogOptions options) noexcept
    {
        auto buttonPressed = ImGui::Button(label);

        auto ptr = &path;
        auto itr = _fileInputResults.find(ptr);
        if (itr != _fileInputResults.end())
        {
            if (itr->second)
            {
                path = itr->second.value()[0];
                _fileInputResults.erase(itr);
                return true;
            }
            return false;
        }

        if (!buttonPressed)
        {
            return false;
        }
        if(options.title.empty())
        {
            options.title = label;
		}
        getWindow().openFileDialog(std::move(options), [this, ptr](const auto& result)
        {
			_fileInputResults[ptr] = result;
        });
        return false;
    }

    std::optional<std::filesystem::path> EditorApp::getSelectedAssetPath() const
    {
        return _inspectorView.getSelectedAssetPath();
    }

    void EditorApp::onAssetPathSelected(const std::filesystem::path& assetPath)
    {
		_inspectorView.selectObject(assetPath);
    }
}