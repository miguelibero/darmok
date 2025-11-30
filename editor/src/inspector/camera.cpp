#include <darmok-editor/inspector/camera.hpp>
#include <darmok-editor/app.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/glm_serialize.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <array>

namespace darmok::editor
{
    std::string CameraInspectorEditor::getTitle() const noexcept
    {
        return "Camera";
    }

    CameraInspectorEditor::RenderResult CameraInspectorEditor::renderType(Camera::Definition& cam) noexcept
    {
        static const std::array<std::string, 2> projOptions
        {
            "Perspective",
            "Orthographic"
        };

        auto changed = false;

        size_t currentProj = cam.has_ortho_center() ? 1 : 0;
        if (ImguiUtils::drawListCombo("Projection", currentProj, projOptions))
        {
            switch (currentProj)
            {
            case 0:
                cam.set_perspective_fovy(glm::radians(60.f));
                break;
            case 1:
                *cam.mutable_ortho_center() = convert<protobuf::Vec2>(glm::vec2{ 0.f });
                break;
            }
            changed = true;
        }
        if (ImguiUtils::beginFrame("Projection Data"))
        {
            if (cam.has_perspective_fovy())
            {
                auto fovy = glm::degrees(cam.perspective_fovy());
                if (ImGui::SliderFloat("Field Of View", &fovy, 0.f, 360.f))
                {
                    changed = true;
                    cam.set_perspective_fovy(glm::radians(fovy));
                }
            }
            else if (cam.has_ortho_center())
            {
                if (ImguiUtils::drawProtobufInput("Center", "ortho_center", cam))
                {
                    changed = true;
                }
            }

            if (ImguiUtils::drawProtobufInput("Near Plane", "near", cam))
            {
                changed = true;
            }
            if (ImguiUtils::drawProtobufInput("Far Plane", "far", cam))
            {
                changed = true;
            }
            ImguiUtils::endFrame();
            if (ImguiUtils::drawProtobufInput("Viewport", "viewport", cam))
            {
                changed = true;
            }
        }

        /*
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
        */
        return changed;
    }
}