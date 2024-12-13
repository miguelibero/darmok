#pragma once

#include <darmok/utils.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

namespace darmok::editor
{
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


        template<typename T>
        static bool drawAssetReference(const char* label, std::shared_ptr<T>& value, const char* dragType = nullptr)
        {
            auto name = value->getName();
            ImGui::BeginDisabled(true);
            ImGui::InputText(label, &name);
            ImGui::EndDisabled();

            dragType = dragType == nullptr ? label : dragType;
            bool changed = false;
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(dragType))
                {
                    IM_ASSERT(payload->DataSize == sizeof(std::shared_ptr<T>));
                    value = *(std::shared_ptr<T>*)payload->Data;
                    changed = true;
                }
                ImGui::EndDragDropTarget();
            }

            return changed;
        }

        static const ImVec2& getAssetSize();

        static bool drawAsset(const char* label, bool selected = false);

        template<typename T>
        static bool drawAsset(const std::shared_ptr<T>& value, bool selected = false, const char* dragType = nullptr)
        {
            auto name = value->getName();
            if (drawAsset(name.c_str(), selected))
            {
                selected = !selected;
            }

            if (dragType != nullptr && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
            {
                auto accepted = ImGui::SetDragDropPayload(dragType, &value, sizeof(std::shared_ptr<T>));
                drawAsset(name.c_str(), accepted);
                ImGui::EndDragDropSource();
            }

            return selected;
        }
    };
}