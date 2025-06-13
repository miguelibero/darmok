#pragma once

#include <darmok/utils.hpp>

#include <stack>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <magic_enum/magic_enum.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace darmok
{
    class Material;
    class Program;
    class IMesh;
    class IMeshLoader;

    namespace protobuf
    {
        class Vec2;
        class Vec3;
        class Quat;
        class Color;
        class Color3;
    }
}

namespace google::protobuf
{
    class Message;
    class FieldDescriptor;
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

    namespace ImguiUtils
    {
        using ComboOptions = std::span<const std::string>;
        bool drawListCombo(const char* label, size_t& current, ComboOptions options) noexcept;

        template<typename T>
        bool drawEnumCombo(const char* label, T& value)
        {
            auto current = static_cast<size_t>(value);
            auto options = StringUtils::getEnumValues<T>();
            if (drawListCombo(label, current, options))
            {
                value = static_cast<T>(current);
                return true;
            }
            return false;
        }

        const ImVec2& getAssetSize();

        bool drawAsset(const char* label, bool selected = false);

        template<typename T>
        ReferenceInputAction drawAssetReference(const char* label, T& value, std::string name, const char* dragType = nullptr)
        {
            ImGui::BeginDisabled(true);
            ImGui::InputText(label, name);
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

        bool drawFileInput(const char* label, std::filesystem::path& path, std::string_view filter = {}) noexcept;

        using FieldDescriptor = google::protobuf::FieldDescriptor;
        using Message = google::protobuf::Message;

        bool drawProtobufInputs(const std::unordered_map<std::string, std::string>& labels, Message& msg) noexcept;
        bool drawProtobufInput(const char* label, const FieldDescriptor& field,Message& msg) noexcept;
        bool drawProtobufInput(const char* label, const char* field, Message& msg) noexcept;
        bool drawProtobufEnumInput(const char* label, const FieldDescriptor& field, Message& msg, std::optional<ComboOptions> options = std::nullopt) noexcept;
        bool drawProtobufEnumInput(const char* label, const char* field, Message& msg, std::optional<ComboOptions> options = std::nullopt) noexcept;

        void beginFrame(const char* name) noexcept;
        void endFrame() noexcept;

        ImVec2 addCursorPos(const ImVec2& delta) noexcept;
        ImVec2 addFrameSpacing(const ImVec2& delta) noexcept;
        ConfirmPopupAction drawConfirmPopup(const char* name, const char* text) noexcept;
    };
}