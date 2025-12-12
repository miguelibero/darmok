#include <darmok-editor/app.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok-editor/IconsMaterialDesign.h>

#include <darmok/window.hpp>
#include <darmok/transform.hpp>

#include <darmok/camera.hpp>
#include <darmok/light.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/physics3d.hpp>
#include <darmok/physics3d_character.hpp>
#include <darmok/stream.hpp>

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

    expected<int32_t, std::string> EditorApp::setup(const CmdArgs& args) noexcept
    {
        auto inspectorResult = _inspectorView.setup();
        if (!inspectorResult)
        {
            return unexpected{ std::move(inspectorResult).error() };
        }

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

        return 0;
    }

    expected<void, std::string> EditorApp::init() noexcept
    {
        auto& win = _app.getWindow();
        win.requestTitle("darmok editor");
        _app.setDebugFlag(BGFX_DEBUG_TEXT);

        auto imguiResult = _app.getOrAddComponent<ImguiAppComponent>(*this);
        if(!imguiResult)
        {
            return unexpected{"failed to add imgui component: " + imguiResult.error()};
		}
        _imgui = imguiResult.value();
        
        auto projResult = _proj.init(_progCompConfig);
        if(!projResult)
        {
            return unexpected{"failed to initialize project: " + projResult.error()};
		}
        auto sceneResult = _sceneView.init(_proj.getScene(), _proj.getCamera().value());
        if (!sceneResult)
        {
            return sceneResult;
        }
        auto inspectorResult = _inspectorView.init(*this);
        if (!inspectorResult)
        {
            return inspectorResult;
        }
        auto assetsResult = _assetsView.init(_proj.getSceneDefinition(), *this);
        if (!assetsResult)
        {
            return assetsResult;
        }
        return {};
    }

    expected<void, std::string> EditorApp::shutdown() noexcept
    {
        stopScene();
        auto result = _inspectorView.shutdown();
        if (!result)
        {
            return result;
        }
        result = _assetsView.shutdown();
        if (!result)
        {
            return result;
        }
        result = _sceneView.shutdown();
        if (!result)
        {
            return result;
        }
        result = _proj.shutdown();
        if (!result)
        {
            return result;
        }
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

        return {};
    }

    expected<void, std::string> EditorApp::imguiSetup() noexcept
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

        return {};
    }

    const ImGuiWindowFlags EditorApp::_fixedFlags = ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    void EditorApp::renderDockspace() noexcept
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

    void EditorApp::playScene() noexcept
    {
        _scenePlaying = true;
    }

    void EditorApp::stopScene() noexcept
    {
        _scenePlaying = false;
    }

    void EditorApp::pauseScene() noexcept
    {
    }

    EntityId EditorApp::getSelectedEntity() const noexcept
    {
        return _inspectorView.getSelectedEntity();
    }

    expected<void, std::string> EditorApp::renderMainMenu() noexcept
    {
        expected<void, std::string> result;
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New", "Ctrl+N"))
                {
                    result = _proj.resetScene();
                }
                if (ImGui::MenuItem("Open...", "Ctrl+O"))
                {
                    result = _proj.openScene();
                }
                if (ImGui::MenuItem("Save", "Ctrl+S"))
                {
                    result = _proj.saveScene();
                }
                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
                {
                    result = _proj.saveScene(true);
                }
                if (ImGui::MenuItem("Export..."))
                {
                    result = _proj.exportScene();
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
                    auto entityResult = _proj.addEntity(_inspectorView.getSelectedEntity());
                    if (!entityResult)
                    {
                        result = unexpected{ std::move(entityResult).error() };
                    }
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
                    if (ImGui::BeginMenu("Physics3d"))
                    {
                        drawEntityComponentMenu("Body", physics3d::PhysicsBody::createDefinition());
                        drawEntityComponentMenu("Character Controller", physics3d::CharacterController::createDefinition());
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
        return result;
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

    void EditorApp::renderAboutDialog() noexcept
    {
    }

    expected<void, std::string> EditorApp::renderMainToolbar() noexcept
    {
        expected<void, std::string> result;
             
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
                result = _proj.saveScene();
            }
            ImGui::SameLine();
            if (ImGui::ButtonEx(ICON_MD_FOLDER_OPEN))
            {
                result = _proj.openScene();
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

        return {};
    }

    void EditorApp::onSceneTreeSceneClicked() noexcept
    {
        onObjectSelected(nullEntityId);
    }

    void EditorApp::onObjectSelected(const SelectableObject& obj) noexcept
    {
        _inspectorView.selectObject(obj);
        auto entityId = _inspectorView.getSelectedEntity();
        auto entity = _proj.getComponentLoadContext().getEntity(entityId);
        _sceneView.selectEntity(entity);
    }

    void EditorApp::onSceneTreeTransformClicked(EntityId entity) noexcept
    {
        onObjectSelected(entity);
    }

    const char* EditorApp::_sceneTreeWindowName = "Scene Tree";

    bool EditorApp::renderSceneTree() noexcept
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
                if (renderEntityDragDropTarget(nullEntityId))
                {
                    changed = true;
                }
                if (ImGui::IsItemClicked())
                {
                    onSceneTreeSceneClicked();
                }
                for (auto entityId : scene.getRootEntities())
                {
                    if (renderSceneTreeBranch(entityId))
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
                    onObjectSelected(nullEntityId);
                    changed = true;
                }
            }
        }
        ImGui::End();
        return changed;
    }

    bool EditorApp::renderSceneTreeBranch(EntityId entity) noexcept
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

    bool EditorApp::renderEntityDragDropTarget(EntityId entity) noexcept
    {
        if (!ImGui::BeginDragDropTarget())
        {
            return false;
        }
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(entityDragType);
        bool changed = false;
        if (payload)
        {
            IM_ASSERT(payload->DataSize == sizeof(EntityId));
            auto droppedEntity = *static_cast<EntityId*>(payload->Data);
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

    expected<void, std::string> EditorApp::imguiRender() noexcept
    {
        auto changed = false;
        auto result = _sceneView.beforeRender();
        if (!result)
        {
            return result;
        }
        result = renderMainMenu();
        if (!result)
        {
            return result;
        }
        renderDockspace();
        result = renderMainToolbar();
        if (!result)
        {
            return result;
        }
        renderSceneTree();
        auto boolResult = _inspectorView.render();
        if (!boolResult)
        {
            return unexpected<std::string>{ "failed to render inspector: " + boolResult.error() };
        }

        result = _proj.render();
        if (!result)
        {
            return unexpected<std::string>{ "failed to render project: " + result.error() };
        }
        result = _sceneView.render();
        if (!result)
        {
            return unexpected<std::string>{ "failed to render scene: " + result.error() };
        }
        boolResult = _assetsView.render();
        if (!boolResult)
        {
            return unexpected<std::string>{ "failed to render assets: " + boolResult.error() };
        }
        return {};
    }

    expected<void, std::string> EditorApp::update(float deltaTime) noexcept
    {
        return _sceneView.update(deltaTime);
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
		getWindow().openFileDialog(std::move(options), [this, ptr](const auto& result) -> expected<void, std::string>
        {
			_fileInputResults[ptr] = result;
            return {};
        });
        return false;
    }

    std::optional<std::filesystem::path> EditorApp::getSelectedAssetPath() const noexcept
    {
        return _inspectorView.getSelectedAssetPath();
    }

    void EditorApp::onAssetPathSelected(const std::filesystem::path& assetPath) noexcept
    {
		_inspectorView.selectObject(assetPath);
    }

    void EditorApp::onAssetFolderEntered(const std::filesystem::path& assetPath) noexcept
    {
    }
}