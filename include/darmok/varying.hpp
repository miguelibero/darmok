#pragma once

#include <darmok/export.h>
#include <darmok/asset_core.hpp>
#include <bgfx/bgfx.h>
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <vector>

namespace darmok
{
    enum class AttribGroup
    {
        Position,
        Normal,
        Tangent,
        Bitangent,
        Color,
        Skinning,
        Texture,
        Count
    };

    enum class VaryingDefinitionFormat
    {
        Binary,
        Json,
        Xml,
    };

    using AttribDefines = std::unordered_set<std::string>;
    using AttribGroups = std::unordered_set<AttribGroup>;

    struct DARMOK_EXPORT AttribUtils final
    {
        static [[nodiscard]] size_t getDisabledGroups(const AttribDefines& defines, AttribGroups& disabledGroups) noexcept;
        static [[nodiscard]] AttribGroup getGroup(bgfx::Attrib::Enum attrib) noexcept;
        static [[nodiscard]] bgfx::Attrib::Enum getBgfx(std::string_view name) noexcept;
        static [[nodiscard]] bgfx::AttribType::Enum getBgfxType(std::string_view name) noexcept;

        static [[nodiscard]] std::string getBgfxName(bgfx::Attrib::Enum val) noexcept;
        static [[nodiscard]] std::string getBgfxTypeName(bgfx::AttribType::Enum val) noexcept;

        static [[nodiscard]] VaryingDefinitionFormat getPathFormat(const std::filesystem::path& path) noexcept;
    };

    struct DARMOK_EXPORT VertexAttribute final
    {
        bgfx::Attrib::Enum attrib = bgfx::Attrib::Count;
        bgfx::AttribType::Enum type = bgfx::AttribType::Float;
        uint8_t num = 1;
        bool normalize = false;
        bool asInt = false;

        void read(const std::string& key);
        void read(const std::string& key, const nlohmann::json& json);
        std::string write(nlohmann::json& json) const noexcept;
        void addTo(bgfx::VertexLayout& layout) const noexcept;
        bool inGroup(AttribGroup group) const noexcept;
        bool inGroups(const AttribGroups& groups) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(attrib, type, num, normalize, asInt);
        }
    };

    struct DARMOK_EXPORT FragmentAttribute final
    {
        bgfx::Attrib::Enum attrib = bgfx::Attrib::Count;
        uint8_t num = 1;
        std::vector<float> defaultValue;

        void read(const std::string& key);
        void read(const std::string& key, const nlohmann::json& json);
        std::string write(nlohmann::json& json) const noexcept;
        bool inGroup(AttribGroup group) const noexcept;
        bool inGroups(const AttribGroups& groups) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(attrib, num, defaultValue);
        }
    };

    struct DARMOK_EXPORT VertexLayout final
    {
        using Format = VaryingDefinitionFormat;
        using Attribute = VertexAttribute;
        using ConstIterator = std::vector<Attribute>::const_iterator;

        VertexLayout(const std::vector<Attribute>& attribs = {}) noexcept;
        VertexLayout(const bgfx::VertexLayout& layout) noexcept;

        bool empty() const noexcept;
        bool has(bgfx::Attrib::Enum attrib) const noexcept;
        ConstIterator begin() const noexcept;
        ConstIterator end() const noexcept;

        bgfx::VertexLayout getBgfx(const AttribGroups& disabledGroups = {}) const noexcept;
        bgfx::VertexLayout getBgfx(const AttribDefines& defines) const noexcept;
        void setBgfx(const bgfx::VertexLayout& layout) noexcept;

        VertexLayout& operator=(const bgfx::VertexLayout& layout) noexcept;
        operator bgfx::VertexLayout() const noexcept;

        void read(const std::filesystem::path& path);
        void write(const std::filesystem::path& path) const noexcept;
        void read(std::istream& in, Format format = Format::Binary);
        void write(std::ostream& out, Format format = Format::Binary) const noexcept;
        void read(const nlohmann::ordered_json& json);
        void write(nlohmann::ordered_json& json) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(_attributes);
        }
    private:
        std::vector<Attribute> _attributes;
    };

    struct DARMOK_EXPORT FragmentLayout final
    {
        using Format = VaryingDefinitionFormat;
        using Attribute = FragmentAttribute;
        using ConstIterator = std::vector<Attribute>::const_iterator;

        FragmentLayout(const std::vector<Attribute>& attribs = {}) noexcept;

        bool empty() const noexcept;
        bool has(bgfx::Attrib::Enum attrib) const noexcept;
        bgfx::Attrib::Enum getUnusedAttrib() const noexcept;
        ConstIterator begin() const noexcept;
        ConstIterator end() const noexcept;

        void read(const std::filesystem::path& path);
        void write(const std::filesystem::path& path) const noexcept;
        void read(std::istream& in, Format format = Format::Binary);
        void write(std::ostream& out, Format format = Format::Binary) const noexcept;
        void read(const nlohmann::ordered_json& json);
        void write(nlohmann::ordered_json& json) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(_attributes);
        }
    private:
        std::vector<Attribute> _attributes;
    };

    struct DARMOK_EXPORT VaryingDefinition final
    {
        using Format = VaryingDefinitionFormat;

        VertexLayout vertex;
        FragmentLayout fragment;
        uint8_t instanceAttribs = 0;

        void read(const nlohmann::ordered_json& json);
        void write(nlohmann::ordered_json& json) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(vertex, fragment);
        }

        void read(const std::filesystem::path& path);
        void write(const std::filesystem::path& path) const noexcept;
        void read(std::istream& in, Format format = Format::Binary);
        void write(std::ostream& out, Format format = Format::Binary) const noexcept;

        void writeBgfx(std::ostream& out, const AttribGroups& disabledGroups = {}) const noexcept;
        void writeBgfx(std::ostream& out, const AttribDefines& defines) const noexcept;
    private:
        static const std::string _vertexJsonKey;
        static const std::string _fragmentJsonKey;

        static [[nodiscard]] std::string getBgfxTypeName(bgfx::Attrib::Enum val) noexcept;
        static [[nodiscard]] std::string getBgfxVarTypeName(uint8_t num) noexcept;
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
