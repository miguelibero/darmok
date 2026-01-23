#include <darmok-editor/app.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok-editor/IconsMaterialDesign.h>
#include <darmok-editor/physics3d.hpp>
#include <darmok-editor/skeleton.hpp>
#include <darmok-editor/text.hpp>
#include <darmok-editor/rmlui.hpp>

#include <darmok/window.hpp>
#include <darmok/transform.hpp>

#include <darmok/camera.hpp>
#include <darmok/light.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/render_deferred.hpp>
#include <darmok/environment.hpp>
#include <darmok/shadow.hpp>
#include <darmok/culling.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/freelook.hpp>
#include <darmok/stream.hpp>
#include <darmok/lua_script.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <CLI/CLI.hpp>

namespace darmok::editor
{
    EditorApp::EditorApp(App& app) noexcept
        : _app{ app }
        , _sceneView{ *this }
        , _playerView{ *this, app }
        , _proj{app}
        , _dockLeftId{0}
        , _dockRightId{0}
        , _dockCenterId{0}
        , _dockDownId{0}
        , _symbolsFont{nullptr}
        , _mainToolbarHeight{0.f}
    {
        _app.setAssetAbsolutePathsAllowed(true);
    }

    EditorApp::~EditorApp() noexcept = default;

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
        DARMOK_TRY(_sceneView.init(_proj.getScene(), _proj.getCamera().value()));
        DARMOK_TRY(_playerView.init(_proj.getScene()));
        DARMOK_TRY(_inspectorView.init(*this));;
        DARMOK_TRY(_assetsView.init(_proj.getSceneDefinition(), *this));

        std::vector<std::string> compErrors;
        for (auto& comp : _comps)
        {
            auto result = comp->init(*this);
            if (!result)
            {
                compErrors.push_back(std::move(result).error());
            }
        }
        if (!compErrors.empty())
        {
            return StringUtils::joinExpectedErrors(compErrors);
        }

        DARMOK_TRY(addComponent<Physics3dEditorAppComponent>());
        DARMOK_TRY(addComponent<SkeletonEditorAppComponent>());
        DARMOK_TRY(addComponent<TextEditorAppComponent>());
        DARMOK_TRY(addComponent<RmluiEditorAppComponent>());

        return {};
    }

    expected<void, std::string> EditorApp::shutdown() noexcept
    {
        std::vector<std::string> compErrors;
        for (auto& comp : _comps)
        {
            auto result = comp->shutdown();
            if (!result)
            {
                compErrors.push_back(std::move(result).error());
            }
        }
        if (!compErrors.empty())
        {
            return StringUtils::joinExpectedErrors(compErrors);
        }
        DARMOK_TRY(_inspectorView.shutdown());
        DARMOK_TRY(_assetsView.shutdown());
        DARMOK_TRY(_sceneView.shutdown());
        DARMOK_TRY(_playerView.shutdown());
        DARMOK_TRY(_proj.shutdown());

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

    expected<void, std::string> EditorApp::addComponent(std::unique_ptr<IEditorAppComponent> comp) noexcept
    {
        if (_imgui)
        {
            DARMOK_TRY(comp->init(*this));
        }
        _comps.push_back(std::move(comp));
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
            ImGui::DockBuilderDockWindow(_playerView.getWindowName().c_str(), _dockCenterId);
            ImGui::DockBuilderDockWindow(_assetsView.getWindowName().c_str(), _dockDownId);
        }
    }   

    EntityId EditorApp::getSelectedEntity() const noexcept
    {
        return _inspectorView.getSelectedEntity();
    }

    expected<void, std::string> EditorApp::renderMainMenu() noexcept
    {
        auto& scene = _proj.getSceneDefinition();
        expected<void, std::string> result;
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New", "Ctrl+N"))
                {
                    _proj.requestResetScene();
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
                DARMOK_TRY(onMainMenuRender(MainMenuSection::File));
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
                DARMOK_TRY(onMainMenuRender(MainMenuSection::Edit));
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Asset"))
            {
                if (ImGui::BeginMenu("Add"))
                {
                    DARMOK_TRY(drawAssetComponentMenu("Program", Program::createSource()));
                    DARMOK_TRY(drawAssetComponentMenu("Texture", Texture::createSource()));
                    DARMOK_TRY(drawAssetComponentMenu("Mesh", Mesh::createSource()));
                    DARMOK_TRY(drawAssetComponentMenu("Material", Material::createDefinition()));
                    DARMOK_TRY(drawAssetComponentMenu("Scene", SceneLoader::createDefinition(true)));
                    DARMOK_TRY(onMainMenuRender(MainMenuSection::AddAsset));
					ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Component"))
            {
                ImGui::BeginDisabled(getSelectedEntity() == nullEntityId);
                if (ImGui::BeginMenu("Entity Component"))
                {
                    if (ImGui::BeginMenu("Add"))
                    {
                        DARMOK_TRY(drawEntityComponentMenu<Renderable>("Renderable"));
                        DARMOK_TRY(drawEntityComponentMenu<Camera>("Camera"));
                        DARMOK_TRY(drawEntityComponentMenu<LuaScript>("Lua Script"));
                        if (ImGui::BeginMenu("Light"))
                        {
                            DARMOK_TRY(drawEntityComponentMenu<PointLight>("Point Light"));
                            DARMOK_TRY(drawEntityComponentMenu<DirectionalLight>("Directional Light"));
                            DARMOK_TRY(drawEntityComponentMenu<SpotLight>("Spot Light"));
                            DARMOK_TRY(drawEntityComponentMenu<AmbientLight>("Ambient Light"));
                            ImGui::EndMenu();
                        }
                        DARMOK_TRY(onMainMenuRender(MainMenuSection::AddEntityComponent));
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndDisabled();
                if (ImGui::BeginMenu("Scene Component"))
                {
                    if (ImGui::BeginMenu("Add"))
                    {
                        DARMOK_TRY(drawSceneComponentMenu<FreelookController>("Freelook Controller"));
                        DARMOK_TRY(drawSceneComponentMenu<LuaScriptRunner>("Lua Script Runner"));
                        DARMOK_TRY(onMainMenuRender(MainMenuSection::AddSceneComponent));
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }
                ImGui::BeginDisabled(!scene.hasComponent<Camera::Definition>(getSelectedEntity()));
                if (ImGui::BeginMenu("Camera Component"))
                {
                    if (ImGui::BeginMenu("Add"))
                    {
                        if (ImGui::BeginMenu("Renderer"))
                        {
                            DARMOK_TRY(drawCameraComponentMenu<ForwardRenderer>("Forward"));
                            DARMOK_TRY(drawCameraComponentMenu<DeferredRenderer>("Deferred"));
                            DARMOK_TRY(drawCameraComponentMenu<SkyboxRenderer>("Skybox"));
                            DARMOK_TRY(drawCameraComponentMenu<ShadowRenderer>("Shadow"));
                            ImGui::EndMenu();
                        }
                        if (ImGui::BeginMenu("Culling"))
                        {
                            DARMOK_TRY(drawCameraComponentMenu<FrustumCuller>("Frustum Culler"));
                            DARMOK_TRY(drawCameraComponentMenu<OcclusionCuller>("Occlusion Culler"));
                            ImGui::EndMenu();
                        }
                        if (ImGui::BeginMenu("Debug"))
                        {
                            DARMOK_TRY(drawCameraComponentMenu<CullingDebugRenderer>("Culling Debug"));
                            DARMOK_TRY(drawCameraComponentMenu<ShadowDebugRenderer>("Shadow Debug"));
                            ImGui::EndMenu();
                        }
                        DARMOK_TRY(drawCameraComponentMenu<LightingRenderComponent>("Lighting"));
                        DARMOK_TRY(onMainMenuRender(MainMenuSection::AddCameraComponent));
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndDisabled();
                ImGui::EndMenu();
            }
            DARMOK_TRY(onMainMenuRender(MainMenuSection::Main));
            if (ImGui::BeginMenu("Help"))
            {
                DARMOK_TRY(onMainMenuRender(MainMenuSection::Help));
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

    void EditorApp::showError(const std::string& error) noexcept
    {
        _errorPopup = error;
    }

    expected<void, std::string> EditorApp::onMainMenuRender(MainMenuSection section) noexcept
    {
        std::vector<std::string> errors;
        for (auto& comp : _comps)
        {
            auto result = comp->renderMainMenu(section);
            if (!result)
            {
                errors.push_back(std::move(result).error());
            }
        }
        return StringUtils::joinExpectedErrors(errors);
    }

    expected<bool, std::string> EditorApp::drawAssetComponentMenu(const char* name, const google::protobuf::Message& asset) noexcept
    {
        if (ImGui::MenuItem(name))
        {
            _assetsView.addAsset(asset);
            return true;
        }
        return false;
    }

    bool EditorApp::canAddSceneComponent(IdType typeId) const noexcept
    {
        return !_proj.getSceneDefinition().hasSceneComponent(typeId);
    }

    bool EditorApp::canAddEntityComponent(IdType typeId) const noexcept
    {
        auto& scene = _proj.getSceneDefinition();
        auto entity = _inspectorView.getSelectedEntity();
        return entity != nullEntityId && !scene.hasComponent(entity, typeId);
    }

    bool EditorApp::canAddCameraComponent(IdType typeId) const noexcept
    {
        auto& scene = _proj.getSceneDefinition();
        auto entity = _inspectorView.getSelectedEntity();
        auto cam = scene.getComponent<Camera::Definition>(entity);
        if (!cam)
        {
            return false;
        }
        return !CameraDefinitionWrapper{ *cam }.hasComponent(typeId);
    }

    void EditorApp::renderAboutDialog() noexcept
    {
        // TODO
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

        if (!result)
        {
            return result;
        }

        renderSeparator();

        {
            ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_SingleSelect;
            auto ms = ImGui::BeginMultiSelect(flags);

            auto renderOption = [this](const char* label, EditorSceneView::TransformMode mode)
            {
                auto selected = _sceneView.getTransformMode() == mode;
                auto size = ImGui::CalcTextSize(label);
                if (ImGui::Selectable(label, selected, 0, size))
                {
                    _sceneView.setTransformMode(mode);
                }
                ImGui::SameLine();
            };

            renderOption(ICON_MD_BACK_HAND, EditorSceneView::TransformMode::Grab);
            renderOption(ICON_MD_OPEN_WITH, EditorSceneView::TransformMode::Translate);
            renderOption(ICON_MD_3D_ROTATION, EditorSceneView::TransformMode::Rotate);
            renderOption(ICON_MD_SCALE, EditorSceneView::TransformMode::Scale);

            ImGui::EndMultiSelect();
            ImGui::SameLine();
        }

        renderSeparator();

        {
            ImGui::BeginGroup();
            if (_playerView.isPlaying())
            {
                if (ImGui::Button(ICON_MD_STOP))
                {
                    result = _playerView.stop();
                }
            }
            else
            {
                if (ImGui::Button(ICON_MD_PLAY_ARROW))
                {
                    result = _playerView.play();
                }
            }
            ImGui::SameLine();
            ImGui::BeginDisabled(!_playerView.isPlaying());
            auto notPaused = !_playerView.isPaused();
            if (ImguiUtils::drawToggleButton(ICON_MD_PAUSE, &notPaused))
            {
                _playerView.pause();
            }
            ImGui::EndDisabled();
            ImGui::EndGroup();
            ImGui::SameLine();
        }

        ImGui::PopFont();
        _mainToolbarHeight = ImGui::GetWindowSize().y;
        ImGui::End();
       
        return result;
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

    void EditorApp::onSceneTreeEntityClicked(EntityId entityId) noexcept
    {
        onObjectSelected(entityId);
    }

    void EditorApp::onSceneTreeEntityFocused(EntityId entityId) noexcept
    {
        auto entity = _proj.getComponentLoadContext().getEntity(entityId);
        _sceneView.focusEntity(entity);
    }

    const char* EditorApp::_sceneTreeWindowName = "Scene Tree";

    expected<bool, std::string> EditorApp::renderSceneTree() noexcept
    {
        auto& sceneDef = _proj.getSceneDefinition();
        auto changed = false;
        if (ImGui::Begin(_sceneTreeWindowName))
        {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
            if (_inspectorView.isSceneSelected())
            {
                flags |= ImGuiTreeNodeFlags_Selected;
            }
            std::string sceneName = sceneDef.getName();
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
                for (auto entityId : sceneDef.getRootEntities())
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
                auto destroyResult = _proj.destroyEntity(entity);
                if (!destroyResult)
                {
                    return unexpected{ std::move(destroyResult).error() };
                }
                if (destroyResult.value())
                {
                    onObjectSelected(nullEntityId);
                    changed = true;
                }
            }
        }
        ImGui::End();
        return changed;
    }

    bool EditorApp::renderSceneTreeBranch(EntityId entityId) noexcept
    {
        auto& scene = _proj.getSceneDefinition();
        std::string name;
        if (auto trans = scene.getComponent<Transform::Definition>(entityId))
        {
            name = trans->name();
        }
        if (name.empty())
        {
            name = "Entity";
        }
        
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
        auto children = scene.getChildren(entityId);
        if (children.empty())
        {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }
        if (_inspectorView.getSelectedEntity() == entityId)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        auto treeExpanded = ImGui::TreeNodeEx(name.c_str(), flags);
        auto changed = false;

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::TextUnformatted(name.c_str());
            ImGui::SetDragDropPayload(entityDragType, &entityId, sizeof(Entity));
            ImGui::EndDragDropSource();
        }
        else
        {
            if (renderEntityDragDropTarget(entityId))
            {
                changed = true;
            }
        }
        if (treeExpanded)
        {
            if (ImGui::IsItemClicked())
            {
                onSceneTreeEntityClicked(entityId);
            }
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                onSceneTreeEntityFocused(entityId);
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

    bool EditorApp::renderEntityDragDropTarget(EntityId entityId) noexcept
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
            if (droppedEntity != entityId)
            {
                auto& scene = _proj.getSceneDefinition();
                if (auto trans = scene.getComponent<Transform::Definition>(droppedEntity))
                {
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

    void EditorApp::focusNextWindowOnPlaybackChange(bool played) noexcept
    {
        auto change = _playerView.getPlaybackChange();
        if (!change)
        {
            return;
        }
        if (*change == played)
        {
            ImGui::SetNextWindowFocus();
            ImGui::SetNextWindowCollapsed(false);
        }
    }

    expected<void, std::string> EditorApp::imguiRender() noexcept
    {
        auto changed = false;
        DARMOK_TRY_PREFIX(_sceneView.beforeRender(), "scene view: ");
        DARMOK_TRY_PREFIX(renderMainMenu(), "main menu: ");
        renderDockspace();
        DARMOK_TRY_PREFIX(renderMainToolbar(), "main toolbar: ");
        DARMOK_TRY_PREFIX(renderSceneTree(), "scene tree: ");
        auto boolResult = _inspectorView.render();
        if (!boolResult)
        {
            return unexpected<std::string>{ "inspector: " + boolResult.error() };
        }
        if (boolResult.value() && getSelectedEntity() == nullEntityId)
        {
            DARMOK_TRY_PREFIX(_proj.updateScene(), "update scene: ");
        }

        DARMOK_TRY_PREFIX(_proj.render(), "project: ");
        focusNextWindowOnPlaybackChange(false);
        DARMOK_TRY_PREFIX(_sceneView.render(), "scene: ");
        focusNextWindowOnPlaybackChange(true);
        DARMOK_TRY_PREFIX(_playerView.render(), "player: ");
        DARMOK_TRY_PREFIX(_assetsView.render(), "assets: ");

        static const char* errorPopupName = "Darmok Editor Error";
        if (!_errorPopup.empty())
        {
            ImGui::OpenPopup(errorPopupName);
        }
        if (ImguiUtils::drawErrorPopup(errorPopupName, _errorPopup.c_str()))
        {
            _errorPopup.clear();
        }

        return {};
    }

    expected<void, std::string> EditorApp::update(float deltaTime) noexcept
    {
        auto result = _sceneView.update(deltaTime);
        if (!result)
        {
            return result;
        }
        result = _playerView.update(deltaTime);
        if (!result)
        {
            return result;
        }
        return {};
    }

    EditorProject& EditorApp::getProject() noexcept
    {
        return _proj;
    }

    const EditorProject& EditorApp::getProject() const noexcept
    {
        return _proj;
    }

    const EditorInspectorView& EditorApp::getInspectorView() const noexcept
    {
        return _inspectorView;
    }

    EditorInspectorView& EditorApp::getInspectorView() noexcept
    {
        return _inspectorView;
    }

    const EditorSceneView& EditorApp::getSceneView() const noexcept
    {
        return _sceneView;
    }

    EditorSceneView& EditorApp::getSceneView() noexcept
    {
        return _sceneView;
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

    const Input& EditorApp::getInput() const noexcept
    {
        return _app.getInput();
    }

    Input& EditorApp::getInput() noexcept
    {
        return _app.getInput();
    }

    void EditorApp::requestRenderReset() noexcept
    {
        _app.requestRenderReset();
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