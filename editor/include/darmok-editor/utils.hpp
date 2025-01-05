#pragma once

#include <darmok/utils.hpp>

#include <stack>

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace darmok
{
    class Material;
    class Program;
    class IMesh;
    class IMeshLoader;
}

namespace darmok::editor
{
    class EditorApp;

    enum class ReferenceInputAction
    {
        None,
        Changed,
        Visit
    };

    enum class ConfirmPopupAction
    {
        None,
        Ok,
        Cancel
    };

    struct ImguiUtils final
    {
        template<typename T, typename Callback>
        static bool drawEnumCombo(const char* label, T& value, Callback callback, size_t size = toUnderlying(T::Count))
        {
            auto selectedIdx = toUnderlying(value);
            bool changed = false;
            if (ImGui::BeginCombo(label, callback(value).c_str()))
            {
                for (size_t i = 0; i < size; ++i)
                {
                    bool selected = (selectedIdx == i);
                    auto item = (T)i;
                    if (ImGui::Selectable(callback(item).c_str(), selected))
                    {
                        selected = true;
                        value = item;
                        changed = true;
                    }
                    if (selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            return changed;
        }

        template<size_t Size>
        static bool drawArrayCombo(const char* label, size_t& current, const std::array<std::string, Size>& options) noexcept
        {
            auto changed = false;
            if (ImGui::BeginCombo(label, options[current].c_str()))
            {
                for (size_t i = 0; i < options.size(); ++i)
                {
                    auto selected = current == i;
                    if (ImGui::Selectable(options[i].c_str(), selected))
                    {
                        changed = true;
                        selected = true;
                        current = i;
                    }
                    if (selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            return changed;
        }

        static const ImVec2& getAssetSize();

        static bool drawAsset(const char* label, bool selected = false);

        template<typename T>
        static ReferenceInputAction drawAssetReference(const char* label, T& value, std::string name, const char* dragType = nullptr)
        {
            ImGui::BeginDisabled(true);
            ImGui::InputText(label, (char*)name.c_str(), name.size());
            ImGui::EndDisabled();
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && ImGui::IsMouseDoubleClicked(0))
            {
                return ReferenceInputAction::Visit;
            }

            auto action = ReferenceInputAction::None;
            dragType = dragType == nullptr ? label : dragType;
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(dragType))
                {
                    IM_ASSERT(payload->DataSize == sizeof(T));
                    value = *(T*)payload->Data;
                    action = ReferenceInputAction::Changed;
                }
                ImGui::EndDragDropTarget();
            }

            return action;
        }

        static bool drawSizeInput(const char* label, glm::vec3& v) noexcept;
        static bool drawPositionInput(const char* label, glm::vec3& v) noexcept;
        static bool drawRotationInput(const char* label, glm::quat& v) noexcept;
        static bool drawScaleInput(const char* label, glm::vec3& v) noexcept;

        static void beginFrame(const char* name) noexcept;
        static void endFrame() noexcept;

        static ImVec2 addCursorPos(const ImVec2& delta) noexcept;
        static ImVec2 addFrameSpacing(const ImVec2& delta) noexcept;
        static ConfirmPopupAction drawConfirmPopup(const char* name, const char* text) noexcept;

    private:

        static const ImVec2 _frameMargin;
        static const ImVec2 _framePadding;
    };
}