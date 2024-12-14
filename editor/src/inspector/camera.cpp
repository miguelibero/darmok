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
    void CameraInspectorEditor::init(AssetContext& assets, EditorProject& proj, ObjectEditorContainer& editors) noexcept
    {
        _editors = editors;
    }

    bool CameraInspectorEditor::render(Camera& cam) noexcept
    {
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
                ImGui::BeginGroup();
                if (auto persp = std::get_if<CameraPerspectiveData>(&proj))
                {
                    ImGui::SliderFloat("Field Of View", &persp->fovy, 0.F, 360.F);
                    ImGui::InputFloat("Near Plane", &persp->near);
                    ImGui::InputFloat("Far Plane", &persp->far);
                }
                else if (auto ortho = std::get_if<CameraOrthoData>(&proj))
                {
                    ImGui::InputFloat2("Center", glm::value_ptr(ortho->center));
                    ImGui::InputFloat("Near Plane", &ortho->near);
                    ImGui::InputFloat("Far Plane", &ortho->far);
                }
                ImGui::EndGroup();
                cam.setProjection(proj);
                ImGui::Spacing();
            }

            {
                auto vp = cam.getViewport();
                ImGui::InputFloat4("Viewport", glm::value_ptr(vp));
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
                            _editors->render(comp);
                        }
                    }
                }
            }
        }
        return true;
    }
}