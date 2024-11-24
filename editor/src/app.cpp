#include <darmok-editor/app.hpp>
#include <darmok/window.hpp>
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

        auto& imgui = _app.addComponent<darmok::ImguiAppComponent>(*this);
        // ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    }

    void EditorAppDelegate::shutdown()
    {
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