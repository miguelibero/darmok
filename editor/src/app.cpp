#include <darmok-editor/app.hpp>
#include <darmok-editor/IconsMaterialDesign.h>
#include <darmok-editor/transform.hpp>
#include <darmok-editor/camera.hpp>
#include <darmok-editor/light.hpp>
#include <darmok/window.hpp>
#include <darmok/asset.hpp>
#include <darmok/scene.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/freelook.hpp>
#include <darmok/camera.hpp>
#include <darmok/light.hpp>
#include <darmok/transform.hpp>
#include <darmok/environment.hpp>
#include <darmok/texture.hpp>
#include <darmok/shadow.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/render_chain.hpp>
#include <darmok/culling.hpp>
#include <darmok/input.hpp>
#include <darmok/mesh.hpp>
#include <darmok/shape.hpp>
#include <darmok/program.hpp>
#include <darmok/material.hpp>
#include <darmok/stream.hpp>
#include <darmok/reflect.hpp>

#include <cereal/cereal.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <ImGuizmo.h>

#include <portable-file-dialogs.h>

namespace darmok::editor
{
    EditorAppDelegate::EditorAppDelegate(App& app) noexcept
        : _app(app)
        , _dockLeftId(0)
        , _dockRightId(0)
        , _dockCenterId(0)
        , _dockDownId(0)
        , _mouseSceneViewMode(MouseSceneViewMode::None)
        , _sceneViewFocused(false)
        , _symbolsFont(nullptr)
        , _scenePlaying(false)
        , _mainToolbarHeight(0.F)
        , _selectedEntity(entt::null)
        , _selectedScene(false)
        , _transGizmoMode(TransformGizmoMode::Translate)
    {
    }

    EditorAppDelegate::~EditorAppDelegate() noexcept
    {
        // empty on purpose
    }

    void EditorAppDelegate::configureEditorScene(Scene& scene)
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
        cam.setViewportPerspective(60.F, 0.3F, 10000.F);
        cam.addComponent<SkyboxRenderer>(skyboxTex);
        cam.addComponent<GridRenderer>();
        cam.addComponent<LightingRenderComponent>();
        ShadowRendererConfig shadowConfig;
        shadowConfig.cascadeAmount = 3;
        cam.addComponent<ShadowRenderer>(shadowConfig);
        cam.addComponent<ForwardRenderer>();
        cam.addComponent<FrustumCuller>();

        if (_sceneBuffer)
        {
            auto& size = _sceneBuffer->getSize();
            cam.getRenderChain().setOutput(_sceneBuffer);
            cam.setViewport(Viewport(size));
            _app.requestRenderReset();
        }
        else
        {
            cam.setEnabled(false);
        }
        _editorCam = cam;
    }

    void EditorAppDelegate::configureDefaultScene(Scene& scene)
    {
        ShadowRendererConfig shadowConfig;
        shadowConfig.cascadeAmount = 3;

        auto camEntity = scene.createEntity();
        auto& cam = scene.addComponent<Camera>(camEntity)
            .setViewportPerspective(60.F, 0.3F, 1000.F);
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

        // TODO: need to find a way to serialize the mesh and the material in the scene
        auto prog = std::make_shared<Program>(StandardProgramType::Forward);
        auto mesh = MeshData(Cube()).createMesh(prog->getVertexLayout());
        auto mat = std::make_shared<Material>(prog);
        mat->setBaseColor(Colors::white());

        scene.addComponent<Renderable>(cubeEntity, std::move(mesh), mat);
        scene.addComponent<Transform>(cubeEntity)
            .setName("Cube");
    }

    std::optional<int32_t> EditorAppDelegate::setup(const std::vector<std::string>& args) noexcept
    {
        ReflectionUtils::bind();
        _inspectorEditors.add<TransformInspectorEditor>();
        _inspectorEditors.add<CameraInspectorEditor>();
        _inspectorEditors.add<PointLightInspectorEditor>();
        _inspectorEditors.add<DirectionalLightInspectorEditor>();
        _inspectorEditors.add<SpotLightInspectorEditor>();
        _inspectorEditors.add<AmbientLightInspectorEditor>();
        return std::nullopt;
    }

    void EditorAppDelegate::init()
    {
        auto& win = _app.getWindow();
        win.requestTitle("darmok editor");
        _app.setDebugFlag(BGFX_DEBUG_TEXT);

        _imgui = _app.getOrAddComponent<ImguiAppComponent>(*this);
        auto& scenes = _app.getOrAddComponent<SceneAppComponent>();
        _scene = scenes.getScene();
        
        configureEditorScene(*_scene);
        configureDefaultScene(*_scene);

        _inspectorEditors.init(*this);
    }

    void EditorAppDelegate::shutdown()
    {
        stopScene();
        _inspectorEditors.shutdown();

        _scene.reset();
        _sceneBuffer.reset();
        _editorCam.reset();
        _imgui.reset();
        _app.removeComponent<ImguiAppComponent>();
        _app.removeComponent<SceneAppComponent>();
        _selectedEntity = entt::null;
        _selectedScene = false;
        _dockDownId = 0;
        _dockRightId = 0;
        _dockLeftId = 0;
        _dockCenterId = 0;
        _sceneViewFocused = false;
        _mouseSceneViewMode = MouseSceneViewMode::None;
        _symbolsFont = nullptr;
        _scenePath.reset();
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
            ImGui::DockBuilderDockWindow(_inspectorWindowName, _dockRightId);
            ImGui::DockBuilderDockWindow(_sceneViewWindowName, _dockCenterId);
            ImGui::DockBuilderDockWindow(_projectWindowName, _dockDownId);
        }
    }

    bool EditorAppDelegate::shouldCameraRender(const Camera& cam) const noexcept
    {
        return _editorCam.ptr() == &cam;
    }

    bool EditorAppDelegate::shouldEntityBeSerialized(Entity entity) const noexcept
    {
        return !isEditorEntity(entity);
    }

    bool EditorAppDelegate::isEditorEntity(Entity entity) const noexcept
    {
        if (_scene && _editorCam && _scene->getEntity(_editorCam.value()) == entity)
        {
            return true;
        }
        return false;
    }

    const std::vector<std::string> EditorAppDelegate::_sceneDialogFilters =
    {
        "Darmok Scene Files", "*.dsc",
        "Darmok Scene XML Files", "*.dsc.xml",
        "Darmok Scene json Files", "*.dsc.json",
        "All Files", "*"
    };

    void EditorAppDelegate::saveScene(bool forceNewPath)
    {
        if (!_scene)
        {
            return;
        }
        if (!_scenePath || forceNewPath)
        {
            std::string initialPath(".");
            if (_scenePath && std::filesystem::exists(_scenePath.value()))
            {
                initialPath = _scenePath.value().string();
            }
            auto dialog = pfd::save_file("Save Scene", initialPath,
                _sceneDialogFilters, pfd::opt::force_path);

            while (!dialog.ready(1000))
            {
                StreamUtils::logDebug("waiting for save dialog...");
            }
            _scenePath = dialog.result();
        }
        if(!_scenePath)
        {
            return;
        }

        {
            auto& scene = *_scene;
            std::ofstream stream(_scenePath.value());
            auto ext = _scenePath->extension();
            if (ext == ".xml")
            {
                cereal::XMLOutputArchive archive(stream);
                save(archive, scene);
            }
            else if (ext == ".json")
            {
                cereal::JSONOutputArchive archive(stream);
                save(archive, scene);
            }
            else
            {
                cereal::PortableBinaryOutputArchive archive(stream);
                save(archive, scene);
            }
        }
    }

    void EditorAppDelegate::openScene()
    {
        if (!_scene)
        {
            return;
        }
        auto dialog = pfd::open_file("Open Scene", ".",
            _sceneDialogFilters);

        while (!dialog.ready(1000))
        {
            StreamUtils::logDebug("waiting for open dialog...");
        }
        std::filesystem::path scenePath = dialog.result()[0];
        if (!std::filesystem::exists(scenePath))
        {
            return;
        }

        stopScene();
        _editorCam.reset();
        _scene->destroyEntitiesImmediate();

        auto& scene = *_scene;

        {
            std::ifstream stream(scenePath);
            auto ext = scenePath.extension();
            if (ext == ".xml")
            {
                cereal::XMLInputArchive archive(stream);
                load(archive, scene);
            }
            else if (ext == ".json")
            {
                cereal::JSONInputArchive archive(stream);
                load(archive, scene);
            }
            else
            {
                cereal::PortableBinaryInputArchive archive(stream);
                load(archive, scene);
            }
        }
        
        configureEditorScene(scene);
        _scenePath = scenePath;
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

    void EditorAppDelegate::renderMainMenu()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open...", "Ctrl+O"))
                {
                    openScene();
                }
                if (ImGui::MenuItem("Save", "Ctrl+S"))
                {
                    saveScene();
                }
                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
                {
                    saveScene(true);
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
                saveScene();
            }
            ImGui::SameLine();
            if (ImGui::ButtonEx(ICON_MD_FOLDER_OPEN))
            {
                openScene();
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
                auto selected = _transGizmoMode == mode;
                auto size = ImGui::CalcTextSize(label);
                if (ImGui::Selectable(label, selected, 0, size))
                {
                    _transGizmoMode = mode;
                }
                ImGui::SameLine();
            };

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
        _selectedScene = true;
        _selectedEntity = entt::null;
    }

    void EditorAppDelegate::onSceneTreeTransformClicked(Transform& trans)
    {
        if (!_scene)
        {
            return;
        }
        onEntitySelected(_scene->getEntity(trans));
    }

    const char* EditorAppDelegate::_sceneTreeWindowName = "Scene Tree";
    const char* EditorAppDelegate::_sceneViewWindowName = "Scene View";
    const char* EditorAppDelegate::_projectWindowName = "Project";
    const char* EditorAppDelegate::_inspectorWindowName = "Inspector";

    void EditorAppDelegate::onEntitySelected(Entity entity) noexcept
    {
        _selectedEntity = entity;
        _selectedScene = false;
    }

    void EditorAppDelegate::renderSceneTree()
    {
        if (ImGui::Begin(_sceneTreeWindowName))
        {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
            if (_selectedScene)
            {
                flags |= ImGuiTreeNodeFlags_Selected;
            }
            if (ImGui::TreeNodeEx("Scene", flags))
            {
                if (ImGui::IsItemClicked())
                {
                    onSceneTreeSceneClicked();
                }
                _scene->forEachChild([this](auto entity, auto& trans) {
                    if (isEditorEntity(entity))
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
                _scene->destroyEntityImmediate(_selectedEntity);
                _selectedEntity = entt::null;
            }
        }
        ImGui::End();
    }

    void EditorAppDelegate::renderInspector()
    {
        if (ImGui::Begin(_inspectorWindowName))
        {
            if (_scene)
            {
                if (_selectedScene)
                {
                    auto comps = SceneReflectionUtils::getSceneComponents(*_scene);
                    _inspectorEditors.render(comps.begin(), comps.end());
                }
                else if (_selectedEntity != entt::null)
                {
                    auto comps = SceneReflectionUtils::getEntityComponents(*_scene, _selectedEntity);
                    _inspectorEditors.render(comps.begin(), comps.end());
                }
            }

        }
        ImGui::End();
    }

    void EditorAppDelegate::updateSceneSize(const glm::uvec2& size) noexcept
    {
        if (size.x <= 0.F || size.y <= 0.F)
        {
            return;
        }
        if (_sceneBuffer && size == _sceneBuffer->getSize())
        {
            return;
        }
        _sceneBuffer = std::make_shared<FrameBuffer>(size);
        _editorCam->getRenderChain().setOutput(_sceneBuffer);
        _editorCam->setEnabled(true);
        _editorCam->setViewport(Viewport(size));
        // TODO: maby a bit harsh
        _app.requestRenderReset();
    }

    void EditorAppDelegate::renderSceneView()
    {        
        if (ImGui::Begin(_sceneViewWindowName))
        {
            auto size = ImGui::GetContentRegionAvail();
            updateSceneSize(glm::uvec2(size.x, size.y));

            if (_sceneBuffer)
            {
                ImguiTextureData texData(_sceneBuffer->getTexture()->getHandle());
                ImGui::Image(texData, size);
                renderGizmos();
            }

            _sceneViewFocused = ImGui::IsWindowFocused();
            if (ImGui::IsWindowHovered())
            {
                if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
                {
                    _mouseSceneViewMode = MouseSceneViewMode::Look;
                }
                else if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
                {
                    _mouseSceneViewMode = MouseSceneViewMode::Drag;
                }
                else
                {
                    _mouseSceneViewMode = MouseSceneViewMode::None;
                }
            }
            else
            {
                _mouseSceneViewMode = MouseSceneViewMode::None;
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

    void EditorAppDelegate::renderGizmos()
    {
        if (!_editorCam  || !_scene)
        {
            return;
        }

        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 size = ImGui::GetItemRectSize();
        ImGuizmo::SetRect(min.x, min.y, size.x, size.y);
        ImGuizmo::SetGizmoSizeClipSpace(0.2F);
        ImGuizmo::Enable(true);

        auto view = _editorCam->getViewMatrix();
        auto viewPos = min;
        ImVec2 viewSize(100, 100);
        viewPos.x += size.x - viewSize.x;
        ImGuizmo::ViewManipulate(glm::value_ptr(view), 100.0F, viewPos, viewSize, 0x101010DD);

        if (_selectedEntity == entt::null)
        {
            return;
        }

        auto trans = _scene->getComponent<Transform>(_selectedEntity);
        if (!trans)
        {
            return;
        }

        auto worldPos = trans->getWorldPosition();
        if (!_editorCam->isWorldPointVisible(worldPos))
        {
            return;
        }

        auto proj = _editorCam->getProjectionMatrix();
        auto op = ImGuizmo::TRANSLATE;
        switch (_transGizmoMode)
        {
        case TransformGizmoMode::Rotate:
            op = ImGuizmo::ROTATE;
            break;
        case TransformGizmoMode::Scale:
            op = ImGuizmo::SCALE;
            break;
        }

        auto mode = ImGuizmo::LOCAL;
        auto mtx = trans->getLocalMatrix();

        if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj), op, mode, glm::value_ptr(mtx)))
        {
            trans->setLocalMatrix(mtx);
        }
    }

    void EditorAppDelegate::imguiRender()
    {
        ImGuizmo::BeginFrame();

        renderMainMenu();
        renderDockspace();
        renderMainToolbar();
        renderSceneTree();
        renderInspector();
        renderSceneView();
        renderProject();
    }

    void EditorAppDelegate::update(float deltaTime)
    {
        auto trans = _editorCam->getTransform();
        if (!trans)
        {
            return;
        }

        auto& input = _app.getInput();

        static const std::array<InputAxis, 2> lookAxis = {
            InputAxis{
                { MouseInputDir{ MouseAnalog::Position, InputDirType::Left } },
                { MouseInputDir{ MouseAnalog::Position, InputDirType::Right } }
            },
            InputAxis{
                { MouseInputDir{ MouseAnalog::Position, InputDirType::Down } },
                { MouseInputDir{ MouseAnalog::Position, InputDirType::Up } }
            }
        };
        static const std::array<InputAxis, 2> moveAxis = {
            InputAxis{
                { KeyboardInputEvent{ KeyboardKey::Left } },
                { KeyboardInputEvent{ KeyboardKey::Right } }
            },
            InputAxis{
                { KeyboardInputEvent{ KeyboardKey::Down }, MouseInputDir{ MouseAnalog::Scroll, InputDirType::Up } },
                { KeyboardInputEvent{ KeyboardKey::Up }, MouseInputDir{ MouseAnalog::Scroll, InputDirType::Down } }
            }
        };

        auto rot = trans->getRotation();

        if (_mouseSceneViewMode != MouseSceneViewMode::None)
        {
            glm::vec2 look;
            input.getAxis(look, lookAxis);
            look.y = Math::clamp(look.y, -90.F, 90.F);
            look = glm::radians(look);

            if (_mouseSceneViewMode == MouseSceneViewMode::Look)
            {
                rot = glm::quat(glm::vec3(0, look.x, 0)) * rot * glm::quat(glm::vec3(look.y, 0, 0));
                trans->setRotation(rot);
            }
        }

        if (_sceneViewFocused)
        {
            glm::vec2 move;
            input.getAxis(move, moveAxis);
            move *= 0.05; // sensitivity
            auto dir = rot * glm::vec3(move.x, 0.F, move.y);
            trans->setPosition(trans->getPosition() + dir);
        }
    }
}