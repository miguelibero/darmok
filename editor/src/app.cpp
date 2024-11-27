#include <darmok-editor/app.hpp>
#include <darmok/window.hpp>
#include <darmok/asset.hpp>
#include <darmok/scene.hpp>
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
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

namespace darmok::editor
{
    EditorAppDelegate::EditorAppDelegate(App& app)
        : _app(app)
        , _dockLeftId(0)
        , _dockRightId(0)
        , _dockCenterId(0)
        , _dockDownId(0)
    {
    }

    void EditorAppDelegate::configureEditorScene(Scene& scene)
    {
        auto skyboxTex = _app.getAssets().getTextureLoader()("cubemap.ktx");
        auto camEntity = _scene->createEntity();
        auto& cam = _scene->addComponent<Camera>(camEntity);
        _scene->addComponent<Transform>(camEntity)
            .setPosition(glm::vec3(10, 5, -10))
            .lookAt(glm::vec3(0))
            .setName("Editor Camera");
        cam.setViewportPerspective(60.F, 0.3F, 10000.F);
        cam.addComponent<SkyboxRenderer>(skyboxTex);
        cam.addComponent<GridRenderer>();
        cam.addComponent<LightingRenderComponent>();
        ShadowRendererConfig shadowConfig;
        shadowConfig.cascadeAmount = 3;
        cam.addComponent<ShadowRenderer>(shadowConfig);
        cam.addComponent<ForwardRenderer>();
        cam.addComponent<FrustumCuller>();

        // will be enabled once we know the size of the scene view window
        cam.setEnabled(false);
        _editorCam = cam;
    }

    void EditorAppDelegate::configureDefaultScene(Scene& scene)
    {
        ShadowRendererConfig shadowConfig;
        shadowConfig.cascadeAmount = 3;

        auto camEntity = _scene->createEntity();
        auto& cam = _scene->addComponent<Camera>(camEntity)
            .setViewportPerspective(60.F, 0.3F, 1000.F);
        cam.addComponent<ShadowRenderer>(shadowConfig);
        cam.addComponent<LightingRenderComponent>();
        cam.addComponent<ForwardRenderer>();
        cam.addComponent<FrustumCuller>();
        cam.setEnabled(false);
        _scene->addComponent<Transform>(camEntity, glm::vec3(0.F, 1.F, -10.F))
            .setName("Main Camera");
        auto lightEntity = _scene->createEntity();
        auto& light = _scene->addComponent<DirectionalLight>(lightEntity);
        _scene->addComponent<Transform>(lightEntity, glm::vec3(0.F, 3.F, 0.F))
            .setEulerAngles(glm::vec3(50.F, -30.F, 0.F))
            .setName("Directional Light");
    }

    void EditorAppDelegate::init()
    {
        auto& win = _app.getWindow();
        win.requestTitle("darmok editor");
        _app.setDebugFlag(BGFX_DEBUG_TEXT);

        _imgui = _app.addComponent<ImguiAppComponent>(*this);
        _scene = _app.addComponent<SceneAppComponent>().getScene();

        _sceneBuffer = std::make_shared<FrameBuffer>(win.getPixelSize());
        
        configureEditorScene(*_scene);
        configureDefaultScene(*_scene);
    }

    void EditorAppDelegate::shutdown()
    {
        _scene.reset();
        _sceneBuffer.reset();
        _editorCam.reset();
        _imgui.reset();
        _app.removeComponent<ImguiAppComponent>();
        _app.removeComponent<SceneAppComponent>();

        _dockDownId = 0;
        _dockRightId = 0;
        _dockLeftId = 0;
        _dockCenterId = 0;
    }

    void EditorAppDelegate::imguiSetup()
    {
        ImGuiIO& io = ImGui::GetIO();

        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        io.Fonts->Clear();
        io.Fonts->AddFontFromFileTTF("assets/noto.ttf", 20);
        io.Fonts->Build();
    }

    const float EditorAppDelegate::_mainToolbarHeight = 20.F;
    const ImGuiWindowFlags EditorAppDelegate::_fixedFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    void EditorAppDelegate::renderDockspace()
    {
        ImGui::SetNextWindowPos(ImVec2(0, _mainToolbarHeight)); // Optional: Start dockspace at top-left
        auto size = ImGui::GetIO().DisplaySize;
        size.y -= _mainToolbarHeight;
        ImGui::SetNextWindowSize(size); // Optional: Size it to cover the main viewport

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
                }
                if (ImGui::MenuItem("Save", "Ctrl+S"))
                {
                }
                if (ImGui::MenuItem("Close", "Ctrl+W"))
                {
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
        ImGui::SetNextWindowSize(ImVec2(0, _mainToolbarHeight));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
        ImGui::Begin("Main Toolbar", NULL, _fixedFlags);
        ImGui::PopStyleVar();

        ImGui::Button("Toolbar goes here", ImVec2(0, 37));

        ImGui::End();
    }

    void EditorAppDelegate::onSceneTreeTransformClicked(Transform& trans)
    {
        if (!_scene)
        {
            return;
        }
        _selectedEntity = _scene->getEntity(trans);
    }

    void EditorAppDelegate::renderSceneTree()
    {
        static const char* winName = "Scene Tree";
        ImGui::DockBuilderDockWindow(winName, _dockLeftId);
        if (ImGui::Begin(winName))
        {
            if (ImGui::TreeNode("Scene"))
            {
                _scene->forEachChild([this](auto entity, auto& trans) {
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
            ImGui::End();
        }
    }

    void EditorAppDelegate::renderInspector()
    {
        static const char* winName = "Inspector";
        ImGui::DockBuilderDockWindow(winName, _dockRightId);
        if (ImGui::Begin(winName))
        {
            if (_scene && _selectedEntity)
            {
                auto entity = _selectedEntity.value();
                // TODO: use reflection
                if (ImGui::CollapsingHeader("Transform"))
                {
                    if (auto trans = _scene->getComponent<Transform>(entity))
                    {
                        {
                            std::string name = trans->getName();
                            if (ImGui::InputText("Name", &name))
                            {
                                trans->setName(name);
                            }
                        }

                        {
                            auto pos = trans->getPosition();
                            if (ImGui::InputFloat3("Position", glm::value_ptr(pos)))
                            {
                                trans->setPosition(pos);
                            }
                        }
                        {
                            auto rot = trans->getRotation();
                            if (ImGui::InputFloat4("Rotation", glm::value_ptr(rot)))
                            {
                                trans->setRotation(rot);
                            }
                        }
                        {
                            auto scale = trans->getScale();
                            if (ImGui::InputFloat3("Scale", glm::value_ptr(scale)))
                            {
                                trans->setScale(scale);
                            }
                        }

                    }
                }
            }
            ImGui::End();
        }
    }

    void EditorAppDelegate::updateSceneSize(const glm::uvec2& size) noexcept
    {
        if (size.x <= 0.F || size.y <= 0.F)
        {
            return;
        }
        if (size == _sceneBuffer->getSize())
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
        static const char* winName = "Scene View";
        ImGui::DockBuilderDockWindow(winName, _dockCenterId);
        if (ImGui::Begin(winName))
        {
            ImVec2 min = ImGui::GetWindowContentRegionMin();
            ImVec2 max = ImGui::GetWindowContentRegionMax();
            ImVec2 size(max.x - min.x, max.y - min.y);
            updateSceneSize(glm::uvec2(size.x, size.y));

            ImguiTextureData texData(_sceneBuffer->getTexture()->getHandle());
            ImGui::Image(texData, size);

            ImGui::End();
        }
    }

    void EditorAppDelegate::renderProject()
    {
        static const char* winName = "Project";
        ImGui::DockBuilderDockWindow(winName, _dockDownId);
        if (ImGui::Begin(winName))
        {
            ImGui::End();
        }
    }

    void EditorAppDelegate::imguiRender()
    {
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
    }
}