#include <darmok-editor/inspector/camera.hpp>
#include <darmok-editor/app.hpp>
#include <darmok/glm.hpp>
#include <darmok/camera_reflect.hpp>
#include <darmok/render_scene.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace darmok::editor
{
    void CameraInspectorEditor::init(EditorApp& app, ObjectEditorContainer& editors) noexcept
    {
        _editors = editors;
    }

    bool CameraInspectorEditor::renderType(Camera& cam) noexcept
    {
        auto changed = false;
        if (ImGui::CollapsingHeader("Camera"))
        {
            {
                static const char* perspLabel = "Perspective";
                static const char* orthoLabel = "Orthographic";

                auto proj = cam.getProjection();
                const char* label = perspLabel;
                auto isOrtho = std::holds_alternative<CameraOrthoData>(proj);
                if(isOrtho)
                {
                    label = orthoLabel;
                }
                if (ImGui::BeginCombo("Projection", label))
                {
                    if (ImGui::Selectable(perspLabel, !isOrtho) && isOrtho)
                    {
                        proj = CameraPerspectiveData{};
                    }
                    if (ImGui::Selectable(orthoLabel, isOrtho) && !isOrtho)
                    {
                        proj = CameraOrthoData{};
                    }
                    ImGui::EndCombo();
                }
                ImGui::BeginChild("Projection");
                if (auto persp = std::get_if<CameraPerspectiveData>(&proj))
                {
                    if (ImGui::SliderFloat("Field Of View", &persp->fovy, 0.F, 360.F))
                    {
                        changed = true;
                    }
                    if (ImGui::InputFloat("Near Plane", &persp->near))
                    {
                        changed = true;
                    }
                    if (ImGui::InputFloat("Far Plane", &persp->far))
                    {
                        changed = true;
                    }
                }
                else if (auto ortho = std::get_if<CameraOrthoData>(&proj))
                {
                    if (ImGui::InputFloat2("Center", glm::value_ptr(ortho->center)))
                    {
                        changed = true;
                    }
                    if (ImGui::InputFloat("Near Plane", &ortho->near))
                    {
                        changed = true;
                    }
                    if (ImGui::InputFloat("Far Plane", &ortho->far))
                    {
                        changed = true;
                    }
                }
                ImGui::EndChild();
                if (changed)
                {
                    cam.setProjection(proj);

                }
                ImGui::Spacing();
            }

            {
                auto vp = cam.getViewport();
                if (ImGui::InputFloat4("Viewport", glm::value_ptr(vp)))
                {
                    changed = true;
                }
                ImGui::Spacing();
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
                            if (_editors->render(comp))
                            {
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
        return changed;
    }
}