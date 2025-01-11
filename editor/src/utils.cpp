#include <darmok-editor/utils.hpp>
#include <darmok-editor/app.hpp>
#include <darmok/mesh.hpp>
#include <darmok/program.hpp>
#include <darmok/material.hpp>
#include <darmok/asset.hpp>
#include <darmok/stream.hpp>

#include <portable-file-dialogs.h>

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

    bool ImguiUtils::drawFileInput(const char* label, std::filesystem::path& path, const std::string& filter) noexcept
    {
        if (ImGui::Button(label))
        {
            std::vector<std::string> filters;
            if (!filter.empty())
            {
                filters.push_back("");
                filters.push_back(filter);
            }
            auto dialog = pfd::open_file(label, path.string(), filters);
            // TODO: move this to an update function
            while (!dialog.ready(1000))
            {
                StreamUtils::logDebug("waiting for open dialog...");
            }
            if (dialog.result().empty())
            {
                return false;
            }
            auto resultPath = dialog.result()[0];
            if (!resultPath.empty())
            {
                path = resultPath;
                return true;
            }
        }
        return false;
    }

    ConfirmPopupAction ImguiUtils::drawConfirmPopup(const char* name, const char* text) noexcept
    {
        auto action = ConfirmPopupAction::None;

        if (ImGui::BeginPopupModal(name, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("%s", text);
            ImGui::Separator();

            if (ImGui::Button("Ok"))
            {
                ImGui::CloseCurrentPopup();
                action = ConfirmPopupAction::Ok;
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
                action = ConfirmPopupAction::Cancel;
            }

            ImGui::EndPopup();
        }
        return action;
    }

    ImVec2 ImguiUtils::addCursorPos(const ImVec2& delta) noexcept
    {
        auto pos = ImGui::GetCursorPos();
        pos = ImVec2(pos.x + delta.x, pos.y + delta.y);
        ImGui::SetCursorPos(pos);
        return pos;
    }

    ImVec2 ImguiUtils::addFrameSpacing(const ImVec2& delta) noexcept
    {
        auto pos = addCursorPos(delta);
        auto size = ImGui::GetContentRegionAvail();
        size.x -= delta.x * 2.F;
        ImGui::SetNextItemWidth(size.x);
        return pos;
    }

    const ImVec2 ImguiUtils::_frameMargin = { 0.F, 10.F };
    const ImVec2 ImguiUtils::_framePadding = { 10.F, 10.F };

    void ImguiUtils::beginFrame(const char* name) noexcept
    {
        addFrameSpacing(_frameMargin);
        addFrameSpacing(_framePadding);

        ImGui::BeginGroup();

        ImGui::Text("%s", name);
    }

    void ImguiUtils::endFrame() noexcept
    {
        ImGui::EndGroup();

        auto start = ImGui::GetItemRectMin();
        start.x -= _framePadding.x;
        start.y -= _framePadding.y;

        auto end = ImGui::GetItemRectMax();
        end.x += _framePadding.x;
        end.y += _framePadding.y;

        ImGui::GetWindowDrawList()->AddRect(start, end, IM_COL32(40, 40, 40, 255));

        addCursorPos(ImVec2{ 0, _framePadding.y });
        addCursorPos(ImVec2{ 0, _frameMargin.y });
    }
}