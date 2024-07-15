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

    using AttribDefines = std::unordered_set<std::string>;
    using AttribGroups = std::unordered_set<AttribGroup>;

    struct DARMOK_EXPORT AttribUtils final
    {
        static [[nodiscard]] size_t getDisabledGroups(const AttribDefines& defines, AttribGroups& disabledGroups) noexcept;
        static [[nodiscard]] AttribGroup getGroup(bgfx::Attrib::Enum attrib) noexcept;
        static [[nodiscard]] bgfx::Attrib::Enum getBgfx(const std::string_view name) noexcept;
        static [[nodiscard]] bgfx::AttribType::Enum getBgfxType(const std::string_view name) noexcept;

        static [[nodiscard]] std::string getBgfxName(bgfx::Attrib::Enum val) noexcept;
        static [[nodiscard]] std::string getBgfxTypeName(bgfx::AttribType::Enum val) noexcept;
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
        std::vector<VertexAttribute> attributes;

        VertexLayout(const std::vector<VertexAttribute>& attribs = {}) noexcept;
        VertexLayout(const bgfx::VertexLayout& layout) noexcept;

        bgfx::VertexLayout getBgfx(const AttribGroups& disabledGroups = {}) const noexcept;
        bgfx::VertexLayout getBgfx(const AttribDefines& defines = {}) const noexcept;
        void setBgfx(const bgfx::VertexLayout& layout) noexcept;

        VertexLayout& operator=(const bgfx::VertexLayout& layout) noexcept;
        operator bgfx::VertexLayout() const noexcept;

        void read(const nlohmann::ordered_json& json);
        void write(nlohmann::ordered_json& json) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(attributes);
        }
    };

    struct DARMOK_EXPORT VaryingDefinition final
    {
        VertexLayout vertex;
        std::vector<FragmentAttribute> fragment;

        void read(const nlohmann::ordered_json& json);
        void write(nlohmann::ordered_json& json) const noexcept;
        void readBgfx(std::istream& in);
        void writeBgfx(std::ostream& out, const AttribGroups& disabledGroups = {}) const noexcept;
        void writeBgfx(std::ostream& out, const AttribDefines& defines = {}) const noexcept;
    private:

        void readFragments(const nlohmann::ordered_json& json);

        static const std::unordered_map<std::string, bgfx::Attrib::Enum>& getAttrs() noexcept;
        static const std::string _vertexJsonKey;
        static const std::string _fragmentJsonKey;

        static const std::string _bgfxVertMarker;
        static const std::string _bgfxFragMarker;
        static const std::string _bgfxInstMarker;
        static const std::string _bgfxInstrEnd;
        static const std::string _bgfxComment;
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
