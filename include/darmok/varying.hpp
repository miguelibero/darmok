#pragma once

#include <darmok/export.h>
#include <darmok/asset_core.hpp>
#include <darmok/serialize.hpp>

#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <vector>
#include <array>

#include <bgfx/bgfx.h>
#include <nlohmann/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>

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

    using AttribDefines = std::unordered_set<std::string>;
    using AttribGroups = std::unordered_set<AttribGroup>;
    using VaryingDefinitionFormat = CerealFormat;

    struct DARMOK_EXPORT AttribUtils final
    {
        static size_t getDisabledGroups(const AttribDefines& defines, AttribGroups& disabledGroups) noexcept;
        [[nodiscard]] static AttribGroup getGroup(bgfx::Attrib::Enum attrib) noexcept;
        [[nodiscard]] static bgfx::Attrib::Enum getBgfx(std::string_view name) noexcept;
        [[nodiscard]] static bgfx::AttribType::Enum getBgfxType(std::string_view name) noexcept;

        [[nodiscard]] static std::string getBgfxName(bgfx::Attrib::Enum val) noexcept;
        [[nodiscard]] static std::string getBgfxTypeName(bgfx::AttribType::Enum val) noexcept;
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

        bool operator==(const VertexAttribute& other) const noexcept;
        bool operator!=(const VertexAttribute& other) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(attrib),
                CEREAL_NVP(type),
                CEREAL_NVP(num),
                CEREAL_NVP(normalize),
                CEREAL_NVP(asInt)
            );
        }
    };

    struct DARMOK_EXPORT FragmentAttribute final
    {
        bgfx::Attrib::Enum attrib = bgfx::Attrib::Count;
        uint8_t num = 1;
        std::string name;
        std::vector<float> defaultValue;

        void read(const std::string& key);
        void read(const std::string& key, const nlohmann::json& json);
        std::string write(nlohmann::json& json) const noexcept;
        bool inGroup(AttribGroup group) const noexcept;
        bool inGroups(const AttribGroups& groups) const noexcept;

        bool operator==(const FragmentAttribute& other) const noexcept;
        bool operator!=(const FragmentAttribute& other) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(attrib),
                CEREAL_NVP(num),
                CEREAL_NVP(defaultValue)
            );
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

        bool operator==(const VertexLayout& other) const noexcept;
        bool operator!=(const VertexLayout& other) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP_("attributes", _attributes));
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
        bgfx::Attrib::Enum getUnusedAttrib(const std::vector<bgfx::Attrib::Enum>& used = {}) const noexcept;
        ConstIterator begin() const noexcept;
        ConstIterator end() const noexcept;

        void read(const std::filesystem::path& path);
        void write(const std::filesystem::path& path) const noexcept;
        void read(std::istream& in, Format format = Format::Binary);
        void write(std::ostream& out, Format format = Format::Binary) const noexcept;
        void read(const nlohmann::ordered_json& json);
        void write(nlohmann::ordered_json& json) const noexcept;

        bool operator==(const FragmentLayout& other) const noexcept;
        bool operator!=(const FragmentLayout& other) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP_("attributes", _attributes));
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
        bool empty() const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(vertex),
                CEREAL_NVP(fragment)
            );
        }

        void read(const std::filesystem::path& path);
        void write(const std::filesystem::path& path) const noexcept;
        void read(std::istream& in, Format format = Format::Binary);
        void write(std::ostream& out, Format format = Format::Binary) const noexcept;

        void writeBgfx(std::ostream& out, const AttribGroups& disabledGroups = {}) const noexcept;
        void writeBgfx(std::ostream& out, const AttribDefines& defines) const noexcept;
        void writeBgfx(const std::filesystem::path& path) const noexcept;

        bool operator==(const VaryingDefinition& other) const noexcept;
        bool operator!=(const VaryingDefinition& other) const noexcept;

    private:
        static const std::string _vertexJsonKey;
        static const std::string _fragmentJsonKey;

        [[nodiscard]] static std::string getBgfxTypeName(bgfx::Attrib::Enum val) noexcept;
        [[nodiscard]] static std::string getBgfxVarTypeName(uint8_t num) noexcept;
    };
}

namespace bgfx
{
    // TODO: check if bgfx::read/write VertexLayout can be accessed
    // https://github.com/bkaradzic/bgfx/blob/master/src/vertexlayout.h#L39
    // instead of creating custom serialization

    struct VertexLayoutElement final
    {
        size_t position;
        uint16_t offset;
        uint16_t attr;

        template<typename Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(position),
                CEREAL_NVP(offset),
                CEREAL_NVP(attr)
            );
        }
    };

    using VertexLayoutElements = std::vector<VertexLayoutElement>;

    template<typename Archive>
    void save(Archive& archive, const VertexLayout& layout)
    {
        VertexLayoutElements elms;
        for (auto i = 0; i < bgfx::Attrib::Count; ++i)
        {
            auto& offset = layout.m_offset[i];
            auto& attr = layout.m_attributes[i];
            if (offset == 0 && attr == 65535)
            {
                continue;
            }
            elms.emplace_back(i, offset, attr);
        }
        archive(
            CEREAL_NVP_("hash", layout.m_hash),
            CEREAL_NVP_("stride", layout.m_stride),
            CEREAL_NVP_("elements", elms)
        );
    }

    template<typename Archive>
    void load(Archive& archive, VertexLayout& layout)
    {
        VertexLayoutElements elms;
        archive(
            CEREAL_NVP_("hash", layout.m_hash),
            CEREAL_NVP_("stride", layout.m_stride),
            CEREAL_NVP_("elements", elms)
        );
        for (auto& elm : elms)
        {
            layout.m_offset[elm.position] = elm.offset;
            layout.m_attributes[elm.position] = elm.attr;
        }
    }

    DARMOK_EXPORT bool operator==(const VertexLayout& a, const VertexLayout& b) noexcept;
    DARMOK_EXPORT bool operator!=(const VertexLayout& a, const VertexLayout& b) noexcept;
}

DARMOK_EXPORT std::string to_string(const bgfx::VertexLayout& layout) noexcept;
DARMOK_EXPORT std::ostream& operator<<(std::ostream& out, const bgfx::VertexLayout& layout) noexcept;

namespace std
{
    template<> struct hash<bgfx::VertexLayout>
    {
        std::size_t operator()(const bgfx::VertexLayout& key) const noexcept;
    };
}