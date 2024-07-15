#pragma once

#include <darmok/export.h>
#include <darmok/asset_core.hpp>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <unordered_map>
#include <filesystem>
#include <glm/glm.hpp>

namespace darmok
{
    enum class ShaderAttributeGroup
    {
        Position,
        Normal,
        Tangent,
        Bitangent,
        Color,
        Skinning,
        Texture
    };

    struct DARMOK_EXPORT VertexAttribute final
    {
        bgfx::Attrib::Enum attrib;
        bgfx::AttribType::Enum type = bgfx::AttribType::Float;
        uint8_t num = 1;
        bool normalize = false;
        bool asInt = false;

        void read(const std::string& key, const nlohmann::json& json);
        std::string write(nlohmann::json& json) const noexcept;
        void addTo(bgfx::VertexLayout& layout) const noexcept;
        bool inGroup(ShaderAttributeGroup group) const noexcept;

        static [[nodiscard]] bool inGroup(bgfx::Attrib::Enum attrib, ShaderAttributeGroup group) noexcept;
        static [[nodiscard]] bgfx::Attrib::Enum getBgfx(const std::string_view name) noexcept;
        static [[nodiscard]] bgfx::AttribType::Enum getBgfxType(const std::string_view name) noexcept;

        static [[nodiscard]] std::string getBgfxName(bgfx::Attrib::Enum val) noexcept;
        static [[nodiscard]] std::string getBgfxTypeName(bgfx::AttribType::Enum val) noexcept;
    };

    struct DARMOK_EXPORT FragmentAttribute final
    {
        bgfx::Attrib::Enum attrib;
        uint8_t num;
        glm::vec4 defaultValue;
    };

    struct DARMOK_EXPORT ProgramAttributes final
    {
        std::vector<VertexAttribute> vertex;
        std::vector<FragmentAttribute> fragment;

        using AttributeGroup = ShaderAttributeGroup;

        bgfx::VertexLayout getVertexLayout(const std::vector<ShaderAttributeGroup>& disabledGroups = {});

        void read(const nlohmann::ordered_json& json) noexcept;
        void write(nlohmann::ordered_json& json) const noexcept;
        void readVaryingDef(std::istream& in);
        void writeVaryingDef(std::ostream& out);
    private:
        static const std::unordered_map<std::string, bgfx::Attrib::Enum>& getVaryingDefAttrs() noexcept;
    };

    struct DARMOK_EXPORT VertexLayoutUtils final
    {
        static void read(const std::filesystem::path& path, bgfx::VertexLayout& layout);
        static void write(const std::filesystem::path& path, const bgfx::VertexLayout& layout);
		static void read(const nlohmann::ordered_json& json, bgfx::VertexLayout& layout);
        static void write(nlohmann::ordered_json& json, const bgfx::VertexLayout& layout);
    };
}

namespace bgfx
{
    // TODO: check if bgfx::read/write VertexLayout can be accessed
    // https://github.com/bkaradzic/bgfx/blob/master/src/vertexlayout.h#L39
    // instead of creating custom serialization

    template<typename Archive>
    void serialize(Archive& archive, VertexLayout& layout)
    {
        archive(
            layout.m_hash,
            layout.m_stride,
            layout.m_offset,
            layout.m_attributes
        );
    }
}

DARMOK_EXPORT std::string to_string(const bgfx::VertexLayout& layout) noexcept;
DARMOK_EXPORT std::ostream& operator<<(std::ostream& out, const bgfx::VertexLayout& layout) noexcept;
