#pragma once

#include <darmok/utils.hpp>

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
    };
}