#include <darmok-editor/camera.hpp>
#include <darmok-editor/app.hpp>
#include <darmok/glm.hpp>
#include <darmok/camera.hpp>
#include <darmok/camera_reflect.hpp>
#include <darmok/render_scene.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace darmok::editor
{
    void CameraInspectorEditor::init(EditorAppDelegate& app, ObjectEditorContainer& editors) noexcept
    {
        _editors = editors;
    }

    bool CameraInspectorEditor::render(Camera& cam) noexcept
    {

        if (ImGui::CollapsingHeader("Camera"))
        {
            {
                std::string name = cam.getName();
                if (ImGui::InputText("Name", &name))
                {
                    cam.setName(name);
                }
            }

            if (_editors)
            {
                auto comps = CameraReflectionUtils::getCameraComponents(cam);
                if (!comps.empty())
                {
                    if (ImGui::CollapsingHeader("Components"))
                    {
                        for (auto& comp : comps)
                        {
                            _editors->render(comp);
                        }
                    }
                }
            }
        }
        return true;
    }
}