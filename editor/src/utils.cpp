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
#include <fmt/format.h>

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

    void ImguiUtils::drawTexturePreview(const Texture& tex, const glm::vec2& maxSize) noexcept
    {
        ImguiTextureData texData{ tex.getHandle() };
        glm::vec2 size{ tex.getSize() };
        auto imAvailSize = ImGui::GetContentRegionAvail();
		glm::vec2 availSize{ imAvailSize.x, imAvailSize.y };
        if(maxSize != glm::vec2{ 0.F })
        {
            availSize = glm::min(availSize, maxSize);
		}
        auto ratio = glm::min(size.x / availSize.x, size.y / availSize.y);
        size /= ratio;
        ImGui::Image(texData, ImVec2{ size.x, size.y });
    }

    void ImguiUtils::drawReferenceInput(const char* label, std::string& value) noexcept
    {
        ImVec4 bg = ImGui::GetStyle().Colors[ImGuiCol_FrameBg];
        bg.w *= 0.5f;
        ImGui::PushStyleColor(ImGuiCol_FrameBg, bg);
        ImGui::InputText(label, &value, ImGuiInputTextFlags_ReadOnly);
        ImGui::PopStyleColor();
	}

    namespace ImguiUtils
    {
        ReferenceInputAction handleAssetReferenceInput(std::string& assetPath, const char* dragType)
        {
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                return ReferenceInputAction::Visit;
            }
            if (ImGui::IsItemActive() && ImGui::IsKeyPressed(ImGuiKey_Delete))
            {
                assetPath.clear();
                return ReferenceInputAction::Changed;
            }

            auto action = ReferenceInputAction::None;
            if(dragType == nullptr)
            {
                return action;
			}
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(dragType))
                {
                    std::string::size_type len = payload->DataSize / sizeof(std::string::value_type);
                    assetPath = std::string{ static_cast<const char*>(payload->Data), len };
                    action = ReferenceInputAction::Changed;
                }
                ImGui::EndDragDropTarget();
            }

            return action;
        }
    }

    ReferenceInputAction ImguiUtils::drawAssetReferenceInput(const char* label, std::string& assetPath, const char* dragType)
    {
        drawReferenceInput(label, assetPath);
		return handleAssetReferenceInput(assetPath, dragType);
    }

    ReferenceInputAction ImguiUtils::drawTextureReferenceInput(const char* label, std::string& assetPath, std::shared_ptr<Texture>& tex, AssetPack& assets, const glm::vec2& maxSize)
    {
        drawReferenceInput(label, assetPath);
        auto action = handleAssetReferenceInput(assetPath, "Texture");
        if(action == ReferenceInputAction::Changed || (!assetPath.empty() && tex == nullptr))
        {
            if (auto result = assets.getTextureLoader()(assetPath))
            {
                tex = result.value();
            }
            else
            {
                tex.reset();
            }
		}
        if (tex)
        {
            drawTexturePreview(*tex, maxSize);
        }
        return action;
    }

    ReferenceInputAction ImguiUtils::drawProtobufAssetReferenceInput(const char* label, const FieldDescriptor& field, Message& msg, const char* dragType) noexcept
    {
        auto refl = msg.GetReflection();
        auto assetPath = refl->GetString(msg, &field);
        auto result = drawAssetReferenceInput(label, assetPath, dragType);
        if (result == ReferenceInputAction::Changed)
        {
            refl->SetString(&msg, &field, assetPath);
        }
        return result;
    }


    ReferenceInputAction ImguiUtils::drawProtobufAssetReferenceInput(const char* label, const char* fieldName, Message& msg, const char* dragType) noexcept
    {
        auto field = msg.GetDescriptor()->FindFieldByName(fieldName);
        if (!field)
        {
            return ReferenceInputAction::None;
        }
        return drawProtobufAssetReferenceInput(label, *field, msg, dragType);
    }


    ReferenceInputAction ImguiUtils::drawEntityReferenceInput(const char* label, Entity& entity, OptionalRef<ConstSceneDefinitionWrapper> sceneDef) noexcept
    {
        std::string name;
        if (entity != entt::null)
        {
            if (sceneDef)
            {
                if (auto trans = sceneDef->getComponent<protobuf::Transform>(entity))
                {
                    name = trans->name();
                }
            }
            if (name.empty())
            {
                name = fmt::format("Missing {}", entt::to_integral(entity));
            }
        }

        drawReferenceInput(label, name);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && ImGui::IsMouseDoubleClicked(0))
        {
            return ReferenceInputAction::Visit;
        }
        if (ImGui::IsItemActive() && ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            entity = entt::null;
            return ReferenceInputAction::Changed;
        }

        auto action = ReferenceInputAction::None;
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EditorApp::entityDragType))
            {
                IM_ASSERT(payload->DataSize == sizeof(Entity));
                entity = *static_cast<Entity*>(payload->Data);
                action = ReferenceInputAction::Changed;
            }
            ImGui::EndDragDropTarget();
        }

        return action;
    }

    ReferenceInputAction ImguiUtils::drawProtobufEntityReferenceInput(const char* label, const FieldDescriptor& field, Message& msg, OptionalRef<ConstSceneDefinitionWrapper> sceneDef) noexcept
    {
        auto refl = msg.GetReflection();
        if (field.cpp_type() != FieldDescriptor::CPPTYPE_UINT32)
        {
            return ReferenceInputAction::None;
        }

        auto entity = Entity{ refl->GetUInt32(msg, &field) };
        auto result = drawEntityReferenceInput(label, entity, sceneDef);
        if (result == ReferenceInputAction::Changed)
        {
            if (entity == entt::null)
            {
                refl->ClearField(&msg, &field);
            }
            else
            {
                refl->SetUInt32(&msg, &field, entt::to_integral(entity));
            }
        }
        return result;
    }

    ReferenceInputAction ImguiUtils::drawProtobufEntityReferenceInput(const char* label, const char* fieldName, Message& msg, OptionalRef<ConstSceneDefinitionWrapper> sceneDef) noexcept
    {
        auto field = msg.GetDescriptor()->FindFieldByName(fieldName);
        if (!field)
        {
            return ReferenceInputAction::None;
        }
        return drawProtobufEntityReferenceInput(label, *field, msg, sceneDef);
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
        std::string currentOption;
        if (current >= 0 && options.size() > current)
        {
            currentOption = options[current];
        }
        if (ImGui::BeginCombo(label, currentOption.c_str()))
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

    bool ImguiUtils::drawProtobufSliderInput(const char* label, const char* fieldName, Message& msg, double min, double max) noexcept
    {
        auto field = msg.GetDescriptor()->FindFieldByName(fieldName);
        if (!field)
        {
            return false;
        }
        return drawProtobufSliderInput(label, *field, msg, min, max);
    }

    bool ImguiUtils::drawProtobufSliderInput(const char* label, const FieldDescriptor& field, Message& msg, double min, double max) noexcept
    {
        const auto* refl = msg.GetReflection();
        switch (field.cpp_type())
        {
        case FieldDescriptor::CPPTYPE_INT32:
        {
            auto v = refl->GetInt32(msg, &field);
            if (ImGui::SliderInt(label, &v, min, max))
            {
                refl->SetInt32(&msg, &field, v);
                return true;
            }
            return false;
        }
        case FieldDescriptor::CPPTYPE_INT64:
        {
            auto v = refl->GetInt64(msg, &field);
            if (ImGui::SliderScalar(label, ImGuiDataType_S64, &v, &min, &max))
            {
                refl->SetInt64(&msg, &field, v);
                return true;
            }
            return false;
        }
        case FieldDescriptor::CPPTYPE_UINT32:
        {
            auto v = refl->GetUInt32(msg, &field);
            if (ImGui::SliderScalar(label, ImGuiDataType_U32, &v, &min, &max))
            {
                refl->SetUInt32(&msg, &field, v);
                return true;
            }
            return false;
        }
        case FieldDescriptor::CPPTYPE_UINT64:
        {
            auto v = refl->GetUInt64(msg, &field);
            if (ImGui::SliderScalar(label, ImGuiDataType_U64, &v, &min, &max))
            {
                refl->SetUInt64(&msg, &field, v);
                return true;
            }
            return false;
        }
        case FieldDescriptor::CPPTYPE_DOUBLE:
        {
            auto v = refl->GetDouble(msg, &field);
            if (ImGui::SliderScalar(label, ImGuiDataType_Double, &v, &min, &max))
            {
                refl->SetDouble(&msg, &field, v);
                return true;
            }
            return false;
        }
        case FieldDescriptor::CPPTYPE_FLOAT:
        {
            auto v = refl->GetFloat(msg, &field);
            if (ImGui::SliderFloat(label, &v, min, max))
            {
                refl->SetFloat(&msg, &field, v);
                return true;
            }
            return false;
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
            defaultOptions = protobuf::getEnumValues(*enumDesc);
            std::transform(defaultOptions.begin(), defaultOptions.end(),
                defaultOptions.begin(), [](auto& v) { StringUtils::camelCaseToHumanReadable(v); return v; });
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

    namespace ImguiUtils
    {
        const float frameIndent = 10.f;
    }

    bool ImguiUtils::beginFrame(const char* name) noexcept
    {
		auto result = ImGui::CollapsingHeader(name, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth);
        if (result)
        {
            ImGui::Indent(ImguiUtils::frameIndent);
        }
        return result;
    }

    void ImguiUtils::endFrame() noexcept
    {
        ImGui::Unindent(ImguiUtils::frameIndent);
    }
}