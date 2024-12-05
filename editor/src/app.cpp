#include <darmok-editor/app.hpp>
#include <darmok-editor/IconsMaterialDesign.h>

#include <darmok/window.hpp>
#include <darmok/transform.hpp>

#include <imgui.h>
#include <imgui_internal.h>

namespace darmok::editor
{
    EditorAppDelegate::EditorAppDelegate(App& app) noexcept
        : _app(app)
        , _sceneView(app)
        , _proj(app)
        , _dockLeftId(0)
        , _dockRightId(0)
        , _dockCenterId(0)
        , _dockDownId(0)
        , _symbolsFont(nullptr)
        , _scenePlaying(false)
        , _mainToolbarHeight(0.F)
        , _selectedEntity(entt::null)
    {
    }

    EditorAppDelegate::~EditorAppDelegate() noexcept
    {
        // empty on purpose
    }    

    std::optional<int32_t> EditorAppDelegate::setup(const std::vector<std::string>& args) noexcept
    {
        ReflectionUtils::bind();
        _inspectorView.setup();
        return std::nullopt;
    }

    void EditorAppDelegate::init()
    {
        auto& win = _app.getWindow();
        win.requestTitle("darmok editor");
        _app.setDebugFlag(BGFX_DEBUG_TEXT);

        _imgui = _app.getOrAddComponent<ImguiAppComponent>(*this);
        
        _proj.init();
        _sceneView.init(_proj.getScene(), _proj.getCamera().value());
        _inspectorView.init(_proj.getScene());
    }

    void EditorAppDelegate::shutdown()
    {
        stopScene();
        _inspectorView.shutdown();

        _proj.shutdown();
        _sceneView.shutdown();
        _imgui.reset();
        _app.removeComponent<ImguiAppComponent>();
        _app.removeComponent<SceneAppComponent>();
        _selectedEntity = entt::null;
        _dockDownId = 0;
        _dockRightId = 0;
        _dockLeftId = 0;
        _dockCenterId = 0;
        _symbolsFont = nullptr;
        _mainToolbarHeight = 0.F;
    }

    void EditorAppDelegate::imguiSetup()
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

    const ImGuiWindowFlags EditorAppDelegate::_fixedFlags = ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    void EditorAppDelegate::renderDockspace()
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
            ImGui::DockBuilderDockWindow(_projectWindowName, _dockDownId);
        }
    }   

    void EditorAppDelegate::playScene()
    {
        _scenePlaying = true;
    }

    void EditorAppDelegate::stopScene()
    {
        _scenePlaying = false;
    }

    void EditorAppDelegate::pauseScene()
    {
    }

    void EditorAppDelegate::addEntityComponent(const entt::meta_type& type)
    {
        if (_selectedEntity == entt::null)
        {
            return;
        }
    }

    void EditorAppDelegate::renderMainMenu()
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
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Component"))
            {
                if (ImGui::BeginMenu("Shape"))
                {
                    if (ImGui::MenuItem("Cube"))
                    {
                    }
                    if (ImGui::MenuItem("Sphere"))
                    {
                    }
                    if (ImGui::MenuItem("Plane"))
                    {
                    }
                    if (ImGui::MenuItem("Capsule"))
                    {
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Light"))
                {
                    if (ImGui::MenuItem("Point Light"))
                    {
                    }
                    if (ImGui::MenuItem("Directional Light"))
                    {
                    }
                    if (ImGui::MenuItem("Spot Light"))
                    {
                    }
                    if (ImGui::MenuItem("Ambient Light"))
                    {
                    }
                }
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

    void EditorAppDelegate::renderMainToolbar()
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

    void EditorAppDelegate::onSceneTreeSceneClicked()
    {
        onEntitySelected(entt::null);
    }

    void EditorAppDelegate::onEntitySelected(Entity entity) noexcept
    {
        _selectedEntity = entity;
        _sceneView.selectEntity(_selectedEntity);
        _inspectorView.selectEntity(_selectedEntity);
    }

    void EditorAppDelegate::onSceneTreeTransformClicked(Transform& trans)
    {
        if (auto scene = _proj.getScene())
        {
            onEntitySelected(scene->getEntity(trans));
        }
    }

    const char* EditorAppDelegate::_sceneTreeWindowName = "Scene Tree";
    const char* EditorAppDelegate::_projectWindowName = "Project";

    void EditorAppDelegate::renderSceneTree()
    {
        auto scene = _proj.getScene();
        if (ImGui::Begin(_sceneTreeWindowName) && scene)
        {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
            if (_selectedEntity == entt::null)
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
                scene->forEachChild([this](auto entity, auto& trans) {
                    if (_proj.isEditorEntity(entity))
                    {
                        return false;
                    }
                    std::string name = trans.getName();
                    if (name.empty())
                    {
                        name = "Entity";
                    }
                    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
                    if (trans.getChildren().empty())
                    {
                        flags |= ImGuiTreeNodeFlags_Leaf;
                    }
                    if (_selectedEntity == entity)
                    {
                        flags |= ImGuiTreeNodeFlags_Selected;
                    }
                    if (ImGui::TreeNodeEx(name.c_str(), flags))
                    {
                        if (ImGui::IsItemClicked())
                        {
                            onSceneTreeTransformClicked(trans);
                        }
                        ImGui::TreePop();
                    }
                    return false;
                });
                ImGui::TreePop();
            }
            if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete) && _selectedEntity != entt::null)
            {
                scene->destroyEntityImmediate(_selectedEntity);
                _selectedEntity = entt::null;
            }
        }
        ImGui::End();
    }

    void EditorAppDelegate::renderProject()
    {
        if (ImGui::Begin(_projectWindowName))
        {
        }
        ImGui::End();
    }

    void EditorAppDelegate::imguiRender()
    {
        _sceneView.beforeRender();
        renderMainMenu();
        renderDockspace();
        renderMainToolbar();
        renderSceneTree();
        _inspectorView.render();
        _sceneView.render();
        renderProject();
    }

    void EditorAppDelegate::update(float deltaTime)
    {
        _sceneView.update(deltaTime);
    }

    
}