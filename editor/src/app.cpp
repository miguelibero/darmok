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

    void EditorAppDelegate::init()
    {
        _app.getWindow().requestTitle("darmok editor");
        _app.setDebugFlag(BGFX_DEBUG_TEXT);

        _imgui = _app.addComponent<ImguiAppComponent>(*this);

        _scene = _app.addComponent<SceneAppComponent>().getScene();

        // editor scene stuff
        auto skyboxTex = _app.getAssets().getTextureLoader()("cubemap.ktx");
        auto editorCamEntity = _scene->createEntity();
        auto& editorCam = _scene->addComponent<Camera>(editorCamEntity);
        _scene->addComponent<Transform>(editorCamEntity)
            .setPosition(glm::vec3(10, 5, -10))
            .lookAt(glm::vec3(0));
        editorCam.setViewportPerspective(60.F, 0.3F, 10000.F);
        editorCam.addComponent<SkyboxRenderer>(skyboxTex);
        editorCam.addComponent<GridRenderer>();

        // default scene
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

    void EditorAppDelegate::shutdown()
    {
        _scene.reset();
    }

    void EditorAppDelegate::imguiSetup()
    {
        ImGuiIO& io = ImGui::GetIO();

        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        io.Fonts->Clear();
        io.Fonts->AddFontFromFileTTF("assets/noto.ttf", 20);
        io.Fonts->Build();
    }

    void EditorAppDelegate::renderDockspace()
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0)); // Optional: Start dockspace at top-left
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize); // Optional: Size it to cover the main viewport

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("DockSpace Window", nullptr, windowFlags);
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
                        name = "GameObject";
                    }
                    if (ImGui::TreeNode(name.c_str()))
                    {
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
            ImGui::End();
        }
    }

    void EditorAppDelegate::renderSceneView()
    {
        static const char* winName = "Scene View";
        ImGui::DockBuilderDockWindow(winName, _dockCenterId);
        if (ImGui::Begin(winName))
        {
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
        renderSceneTree();
        renderInspector();
        renderSceneView();
        renderProject();
    }

    void EditorAppDelegate::update(float deltaTime)
    {
    }
}