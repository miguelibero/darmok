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
#include <imgui_stdlib.h>

namespace darmok::editor
{
    EditorAppDelegate::EditorAppDelegate(App& app)
        : _app(app)
    {
    }

    void EditorAppDelegate::init()
    {
        _app.getWindow().requestTitle("darmok editor");
        _app.setDebugFlag(BGFX_DEBUG_TEXT);

        _imgui = _app.addComponent<ImguiAppComponent>(*this);
        // ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        _scene = _app.addComponent<SceneAppComponent>().getScene();

        // editor scene stuff
        auto skyboxTex = _app.getAssets().getTextureLoader()("cubemap.ktx");
        _scene->addComponent<SkyboxRenderer>(skyboxTex);

        // default scene

        ShadowRendererConfig shadowConfig;
        shadowConfig.cascadeAmount = 3;

        auto camEntity = _scene->createEntity();
        auto& cam = _scene->addComponent<Camera>(camEntity)
            .setViewportPerspective(glm::radians(60), 0.3F, 1000.F);
        cam.addComponent<ShadowRenderer>(shadowConfig);
        cam.addComponent<LightingRenderComponent>();
        cam.addComponent<ForwardRenderer>();
        cam.addComponent<FrustumCuller>();
        _scene->addComponent<Transform>(camEntity, glm::vec3(0.F, 1.F, -10.F))
            .setName("Main Camera");
        auto lightEntity = _scene->createEntity();
        auto& light = _scene->addComponent<DirectionalLight>(lightEntity, glm::vec3(0.F, 3.F, 0.F));
        _scene->addComponent<Transform>(lightEntity, glm::vec3(0.F, 1.F, -10.F))
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
        io.Fonts->Clear();
        io.Fonts->AddFontFromFileTTF("assets/noto.ttf", 20);
        io.Fonts->Build();
    }

    void EditorAppDelegate::imguiRender()
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
        // ImGui::TextWrapped("lala");
    }

    void EditorAppDelegate::update(float deltaTime)
    {

    }
}