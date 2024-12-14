#pragma once

#include <darmok/utils.hpp>

#include <imgui.h>

namespace darmok
{
    class Material;
    class Program;
    class IMesh;
    class IProgramLoader;
    class IMeshLoader;
}

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

        static const ImVec2& getAssetSize();

        static bool drawAsset(const char* label, bool selected = false);

        static bool drawMaterialReference(const char* label, std::shared_ptr<Material>& mat);
        static bool drawProgramReference(const char* label, std::shared_ptr<Program>& prog, IProgramLoader& loader);
        static bool drawMeshReference(const char* label, std::shared_ptr<IMesh>& mesh, IMeshLoader& loader);

    private:

        template<typename T>
        static bool drawAssetReference(const char* label, T& value, std::string name, const char* dragType = nullptr)
        {
            ImGui::BeginDisabled(true);
            ImGui::InputText(label, (char*)name.c_str(), name.size());
            ImGui::EndDisabled();

            dragType = dragType == nullptr ? label : dragType;
            bool changed = false;
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(dragType))
                {
                    IM_ASSERT(payload->DataSize == sizeof(T));
                    value = *(T*)payload->Data;
                    changed = true;
                }
                ImGui::EndDragDropTarget();
            }

            return changed;
        }
    };
}