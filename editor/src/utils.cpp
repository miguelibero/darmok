#include <darmok-editor/utils.hpp>
#include <darmok-editor/app.hpp>
#include <darmok/mesh.hpp>
#include <darmok/program.hpp>
#include <darmok/material.hpp>
#include <darmok/asset.hpp>
#include <darmok/stream.hpp>
#include <darmok/glm_serialize.hpp>

#include <portable-file-dialogs.h>
#include <imgui_stdlib.h>

namespace darmok::editor
{
    const ImVec2& ImguiUtils::getAssetSize()
    {
        static const ImVec2 size = ImVec2{ 100, 100 };
        return size;
    }

    bool ImguiUtils::drawAsset(const char* label, bool selected)
    {
        static const ImGuiSelectableFlags flags = 0;

        auto size = getAssetSize();
        ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2{ 0.5f, 0.5f });
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0, 0 });
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

    bool ImguiUtils::drawFileInput(const char* label, std::filesystem::path& path, std::string_view filter) noexcept
    {
        if (ImGui::Button(label))
        {
            std::vector<std::string> filters;
            if (!filter.empty())
            {
                filters.push_back({});
                filters.push_back(std::string{ filter });
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

    bool ImguiUtils::drawListCombo(const char* label, size_t& current, ComboOptions options) noexcept
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

    bool ImguiUtils::drawProtobufInputs(const std::unordered_map<std::string, std::string>& labels, google::protobuf::Message& msg) noexcept
    {
        auto desc = msg.GetDescriptor();
        bool changed = false;
        for (int i = 0; i < desc->field_count(); ++i)
        {
            const auto* field = desc->field(i);
            if (!field)
            {
                continue;
            }
            auto label = field->name();
            auto itr = labels.find(label);
            if (itr == labels.end())
            {
                continue;
            }
            if (drawProtobufInput(label.c_str(), *field, msg))
            {
                changed = true;
            }
        }
        return changed;
    }

    bool ImguiUtils::drawProtobufInput(const char* label, const char* fieldName, google::protobuf::Message& msg) noexcept
    {
        auto field = msg.GetDescriptor()->FindFieldByName(fieldName);
        if (!field)
        {
            return false;
        }
        return drawProtobufInput(label, *field, msg);
    }

    bool ImguiUtils::drawProtobufInput(const char* label, const FieldDescriptor& field, Message& msg) noexcept
    {
        const auto* refl = msg.GetReflection();
        switch (field.cpp_type())
        {
            case FieldDescriptor::CPPTYPE_INT32:
            {
                auto v = refl->GetInt32(msg, &field);
                if (ImGui::InputInt(label, &v))
                {
                    refl->SetInt32(&msg, &field, v);
                    return true;
                }
                return false;
            }
            case FieldDescriptor::CPPTYPE_INT64:
            {
                auto v = refl->GetInt64(msg, &field);
                if (ImGui::InputScalar(label, ImGuiDataType_S64, &v))
                {
                    refl->SetInt64(&msg, &field, v);
                    return true;
                }
                return false;
            }
            case FieldDescriptor::CPPTYPE_UINT32:
            {
                auto v = refl->GetUInt32(msg, &field);
                if (ImGui::InputScalar(label, ImGuiDataType_U32, &v))
                {
                    refl->SetUInt32(&msg, &field, v);
                    return true;
                }
                return false;
            }
            case FieldDescriptor::CPPTYPE_UINT64:
            {
                auto v = refl->GetUInt64(msg, &field);
                if (ImGui::InputScalar(label, ImGuiDataType_U64, &v))
                {
                    refl->SetUInt64(&msg, &field, v);
                    return true;
                }
                return false;
            }
            case FieldDescriptor::CPPTYPE_DOUBLE:
            {
                auto v = refl->GetDouble(msg, &field);
                if (ImGui::InputDouble(label, &v))
                {
                    refl->SetDouble(&msg, &field, v);
                    return true;
                }
                return false;
            }
            case FieldDescriptor::CPPTYPE_FLOAT:
            {
                auto v = refl->GetFloat(msg, &field);
                if (ImGui::InputFloat(label, &v))
                {
                    refl->SetFloat(&msg, &field, v);
                    return true;
                }
                return false;
            }
            case FieldDescriptor::CPPTYPE_BOOL:
            {
                auto v = refl->GetBool(msg, &field);
                if (ImGui::Checkbox(label, &v))
                {
                    refl->SetBool(&msg, &field, v);
                    return true;
                }
                return false;
            }
            case FieldDescriptor::CPPTYPE_ENUM:
            {
                return drawProtobufEnumInput(label, field, msg);
            }
            case FieldDescriptor::CPPTYPE_STRING:
            {
                auto v = refl->GetString(msg, &field);
                if (ImGui::InputText(label, &v))
                {
                    refl->SetString(&msg, &field, v);
                    return true;
                }
                return false;
            }
            case FieldDescriptor::CPPTYPE_MESSAGE:
            {
                auto fieldMsg = refl->MutableMessage(&msg, &field);
                auto typeId = protobuf::getTypeId(*fieldMsg);
                if(typeId == protobuf::getTypeId<protobuf::Vec2>())
                {
                    auto vec = static_cast<protobuf::Vec2*>(fieldMsg);
                    auto glm = protobuf::convert(*vec);
                    if (ImGui::InputFloat2(label, glm::value_ptr(glm)))
                    {
                        *vec = protobuf::convert(glm);
                        return true;
                    }
                    return false;
                }
                else if (typeId == protobuf::getTypeId<protobuf::Vec3>())
                {
                    auto vec = static_cast<protobuf::Vec3*>(fieldMsg);
                    auto glm = protobuf::convert(*vec);
                    if (ImGui::InputFloat3(label, glm::value_ptr(glm)))
                    {
                        *vec = protobuf::convert(glm);
                        return true;
                    }
                    return false;
                }
                else if (typeId == protobuf::getTypeId<protobuf::Vec4>())
                {
                    auto vec = static_cast<protobuf::Vec4*>(fieldMsg);
                    auto glm = protobuf::convert(*vec);
                    if (ImGui::InputFloat4(label, glm::value_ptr(glm)))
                    {
                        *vec = protobuf::convert(glm);
                        return true;
                    }
                    return false;
                }
                else if (typeId == protobuf::getTypeId<protobuf::Uvec2>())
                {
                    auto vec = static_cast<protobuf::Uvec2*>(fieldMsg);
                    auto glm = protobuf::convert(*vec);
                    if (ImGui::InputScalarN(label, ImGuiDataType_U32, glm::value_ptr(glm), 2))
                    {
                        *vec = protobuf::convert(glm);
                        return true;
                    }
                    return false;
                }
                else if (typeId == protobuf::getTypeId<protobuf::Quat>())
                {
                    auto quat = static_cast<protobuf::Quat*>(fieldMsg);
                    auto glm = protobuf::convert(*quat);
                    auto angles = glm::degrees(glm::eulerAngles(glm));
                    if (ImGui::InputFloat3(label, glm::value_ptr(angles)))
                    {
                        *quat = protobuf::convert(glm::quat{ glm::radians(angles) });
                        return true;
                    }
                    return false;
                }
                else if (typeId == protobuf::getTypeId<protobuf::Color>())
                {
                    auto color = static_cast<protobuf::Color*>(fieldMsg);
                    auto glm = Colors::normalize(protobuf::convert(*color));
                    if (ImGui::ColorEdit4(label, glm::value_ptr(glm)))
                    {
                        *color = protobuf::convert(Colors::denormalize(glm));
                        return true;
                    }
                    return false;
                }
                else if (typeId == protobuf::getTypeId<protobuf::Color3>())
                {
                    auto color = static_cast<protobuf::Color3*>(fieldMsg);
                    auto glm = Colors::normalize(protobuf::convert(*color));
                    if (ImGui::ColorEdit3(label, glm::value_ptr(glm)))
                    {
                        *color = protobuf::convert(Colors::denormalize(glm));
                        return true;
                    }
                    return false;
                }
            }
        }
        return false;
    }

    bool ImguiUtils::drawProtobufEnumInput(const char* label, const char* fieldName, Message& msg, std::optional<ComboOptions> options) noexcept
    {
        auto field = msg.GetDescriptor()->FindFieldByName(fieldName);
        if (!field)
        {
            return false;
        }
        return drawProtobufEnumInput(label, *field, msg, options);
    }

    bool ImguiUtils::drawProtobufEnumInput(const char* label, const FieldDescriptor& field, Message& msg, std::optional<ComboOptions> options) noexcept
    {
        auto refl = msg.GetReflection();
        std::vector<std::string> defaultOptions;
        if (!options)
        {
            auto enumDesc = refl->GetEnum(msg, &field)->type();
            auto prefix = enumDesc->full_name() + "_";
            defaultOptions.reserve(enumDesc->value_count());
            for (int i = 0; i < enumDesc->value_count(); ++i)
            {
                auto value = enumDesc->value(i)->name();
                if (StringUtils::startsWith(value, prefix))
                {
                    value = value.substr(prefix.size());
                    defaultOptions.push_back(std::move(value));
                }
            }
            options = defaultOptions;
        }

        size_t v = refl->GetEnumValue(msg, &field);
        if (drawListCombo(label, v, *options))
        {
            refl->SetEnumValue(&msg, &field, v);
            return true;
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

    namespace ImguiUtils
    {
        const ImVec2 _frameMargin = { 0.F, 10.F };
        const ImVec2 _framePadding = { 10.F, 10.F };
    }

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