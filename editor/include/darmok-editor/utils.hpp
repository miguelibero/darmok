#pragma once

#include <darmok/utils.hpp>
#include <darmok/string.hpp>
#include <darmok/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok-editor/asset_fwd.hpp>

#include <imgui.h>

namespace darmok
{
    class ConstSceneDefinitionWrapper;
    class Texture;
    class AssetPack;

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
            auto enumValues = StringUtils::getEnumValues<T>();
            std::vector<std::string> options;
			options.reserve(enumValues.size());
            for (auto& [val, name] : enumValues)
            {
                options.push_back(name);
            }
            if (drawListCombo(label, current, options))
            {
                auto currentOption = options[current];
                auto itr = std::find_if(enumValues.begin(), enumValues.end(),
					[&currentOption](const auto& pair) { return pair.second == currentOption; });
                if(itr != enumValues.end())
                {
                    value = itr->first;
                    return true;
                }
            }
            return false;
        }

        const ImVec2& getAssetSize();

        bool drawAsset(const char* label, bool selected = false);

        void drawTexturePreview(const Texture& tex, const glm::vec2& maxSize = {}) noexcept;

        using FieldDescriptor = google::protobuf::FieldDescriptor;
        using Message = google::protobuf::Message;

        void drawReferenceInput(const char* label, std::string& value) noexcept;

        ReferenceInputAction drawAssetReferenceInput(const char* label, std::string& assetPath, const char* dragType = nullptr);
        ReferenceInputAction drawProtobufAssetReferenceInput(const char* label, const FieldDescriptor& field, Message& msg, const char* dragType = nullptr) noexcept;
        ReferenceInputAction drawProtobufAssetReferenceInput(const char* label, const char* field, Message& msg, const char* dragType = nullptr) noexcept;

        ReferenceInputAction drawTextureReferenceInput(const char* label, std::string& assetPath, std::shared_ptr<Texture>& tex, AssetPack& assets, const glm::vec2& maxSize = {});

        ReferenceInputAction drawEntityReferenceInput(const char* label, Entity& entity, OptionalRef<ConstSceneDefinitionWrapper> sceneDef = std::nullopt) noexcept;
        ReferenceInputAction drawProtobufEntityReferenceInput(const char* label, const FieldDescriptor& field, Message& msg, OptionalRef<ConstSceneDefinitionWrapper> sceneDef = std::nullopt) noexcept;
        ReferenceInputAction drawProtobufEntityReferenceInput(const char* label, const char* field, Message& msg, OptionalRef<ConstSceneDefinitionWrapper> sceneDef = std::nullopt) noexcept;

        bool drawFileInput(const char* label, std::filesystem::path& path, std::string_view filter = {}) noexcept;
        bool drawProtobufInputs(const std::unordered_map<std::string, std::string>& labels, Message& msg) noexcept;
        bool drawProtobufInput(const char* label, const FieldDescriptor& field, Message& msg) noexcept;
        bool drawProtobufInput(const char* label, const char* field, Message& msg) noexcept;
        bool drawProtobufSliderInput(const char* label, const FieldDescriptor& field, Message& msg, double min, double max) noexcept;
        bool drawProtobufSliderInput(const char* label, const char* field, Message& msg, double min, double max) noexcept;
        bool drawProtobufEnumInput(const char* label, const FieldDescriptor& field, Message& msg, std::optional<ComboOptions> options = std::nullopt) noexcept;
        bool drawProtobufEnumInput(const char* label, const char* field, Message& msg, std::optional<ComboOptions> options = std::nullopt) noexcept;

        bool beginFrame(const char* name) noexcept;
        void endFrame() noexcept;

        ConfirmPopupAction drawConfirmPopup(const char* name, const char* text) noexcept;
    };
}