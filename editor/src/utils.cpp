#include <darmok-editor/utils.hpp>
#include <darmok-editor/app.hpp>
#include <darmok/mesh.hpp>
#include <darmok/program.hpp>
#include <darmok/material.hpp>
#include <darmok/asset.hpp>

namespace darmok::editor
{
    const ImVec2& ImguiUtils::getAssetSize()
    {
        static const ImVec2 size = ImVec2(100, 100);
        return size;
    }

    bool ImguiUtils::drawAsset(const char* label, bool selected)
    {
        static const ImGuiSelectableFlags flags = 0;

        auto size = getAssetSize();
        ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        selected = ImGui::Selectable(label, selected, flags, size);
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();

        auto min = ImGui::GetItemRectMin();
        auto max = ImGui::GetItemRectMax();
        auto borderColor = selected ? IM_COL32(255, 0, 0, 255) : IM_COL32(200, 200, 200, 255);
        auto drawList = ImGui::GetWindowDrawList();
        drawList->AddRect(min, max, borderColor, 0.0f, 0, 2.0f);
        return selected;
    }


    bool ImguiUtils::drawSizeInput(const char* label, glm::vec3& v) noexcept
    {
        return ImGui::InputFloat3(label, glm::value_ptr(v));
    }

    bool ImguiUtils::drawPositionInput(const char* label, glm::vec3& v) noexcept
    {
        return ImGui::InputFloat3(label, glm::value_ptr(v));
    }

    bool ImguiUtils::drawRotationInput(const char* label, glm::quat& v) noexcept
    {
        auto angles = glm::degrees(glm::eulerAngles(v));
        if (ImGui::InputFloat3(label, glm::value_ptr(angles)))
        {
            v = glm::quat(glm::radians(angles));
            return true;
        }
        return false;
    }

    bool ImguiUtils::drawScaleInput(const char* label, glm::vec3& v) noexcept
    {
        return ImGui::InputFloat3(label, glm::value_ptr(v));
    }

    void ImguiUtils::beginFrame(const char* name)
    {
        ImVec2 start = ImGui::GetCursorScreenPos();
        _frames.emplace(start);
        ImGui::BeginGroup();
        ImGui::Text(name);
    }

    void ImguiUtils::endFrame()
    {
        ImGui::EndGroup();
        if (_frames.empty())
        {
            return;
        }
        auto frame = _frames.top();
        _frames.pop();
        frame.end = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddRect(frame.start, frame.end, IM_COL32(255, 0, 0, 255));
    }

    std::stack<ImguiUtils::FrameData> ImguiUtils::_frames;
}