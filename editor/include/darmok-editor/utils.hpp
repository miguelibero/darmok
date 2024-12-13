#pragma once

#include <darmok/utils.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

namespace darmok::editor
{
    template<typename T, typename Callback>
    bool ImguiEnumCombo(const char* label, T& value, Callback callback, size_t size = toUnderlying(T::Count))
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

    template<typename T>
    bool ImguiAssetReference(const char* label, std::shared_ptr<T>& value, const char* type = nullptr)
    {
        auto name = value->getName();
        ImGui::BeginDisabled(true);
        ImGui::InputText(label, &name);
        ImGui::EndDisabled();

        type = type == nullptr ? label : type;
        bool changed = false;
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(type))
            {
                IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<T>));
                value = *(std::shared_ptr<T>*)payload->Data;
                changed = true;
            }
            ImGui::EndDragDropTarget();
        }

        return changed;
    }
}