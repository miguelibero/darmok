#pragma once

#include <darmok/export.h>
#include <darmok/asset_core.hpp>
#include <darmok/expected.hpp>
#include <darmok/protobuf/varying.pb.h>

#include <string>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <vector>
#include <array>

#include <bgfx/bgfx.h>
#include <nlohmann/json.hpp>

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
    using VertexAttribute = protobuf::VertexAttribute;
    using FragmentAttribute = protobuf::FragmentAttribute;
    using VertexLayout = protobuf::VertexLayout;
    using FragmentLayout = protobuf::FragmentLayout;
    using VaryingDefinition = protobuf::Varying;

    namespace AttribUtils
    {
        size_t getDisabledGroups(const AttribDefines& defines, AttribGroups& disabledGroups) noexcept;
        [[nodiscard]] AttribGroup getGroup(bgfx::Attrib::Enum attrib) noexcept;
        [[nodiscard]] bool inGroup(bgfx::Attrib::Enum attrib, AttribGroup group) noexcept;
        [[nodiscard]] bool inGroups(bgfx::Attrib::Enum attrib, const AttribGroups& groups) noexcept;

        [[nodiscard]] bgfx::Attrib::Enum getBgfx(std::string_view name) noexcept;
        [[nodiscard]] bgfx::AttribType::Enum getBgfxType(std::string_view name) noexcept;

        [[nodiscard]] std::string getBgfxName(bgfx::Attrib::Enum val) noexcept;
        [[nodiscard]] std::string getBgfxTypeName(bgfx::AttribType::Enum val) noexcept;
    };

    namespace VaryingUtils
    {
        [[nodiscard]] expected<void, std::string> read(VertexAttribute& attrib, const std::string& key) noexcept;
        [[nodiscard]] expected<void, std::string> read(VertexAttribute& attrib, const std::string& key, const nlohmann::json& json) noexcept;
        [[nodiscard]] expected<void, std::string> read(FragmentAttribute& attrib, const std::string& key) noexcept;
        [[nodiscard]] expected<void, std::string> read(FragmentAttribute& attrib, const std::string& key, const nlohmann::json& json) noexcept;

        void addVertexAttribute(bgfx::VertexLayout& layout, const VertexAttribute& attrib) noexcept;

        [[nodiscard]] bgfx::VertexLayout getBgfx(const VertexLayout& layout, const AttribGroups& disabledGroups = {}) noexcept;
        [[nodiscard]] bgfx::VertexLayout getBgfx(const VertexLayout& layout, const AttribDefines& defines) noexcept;
        void read(VertexLayout& layout, const bgfx::VertexLayout& bgfxLayout) noexcept;
        [[nodiscard]] expected<void, std::string> read(VertexLayout& layout, const nlohmann::ordered_json& json) noexcept;

        [[nodiscard]] bgfx::Attrib::Enum getUnusedAttrib(const FragmentLayout& layout, const std::vector<bgfx::Attrib::Enum>& used = {}) noexcept;
        [[nodiscard]] expected<void, std::string> read(FragmentLayout& layout, const nlohmann::ordered_json& json) noexcept;
        [[nodiscard]] expected<void, std::string> read(VaryingDefinition& varying, const nlohmann::ordered_json& json) noexcept;
        [[nodiscard]] expected<void, std::string> read(VaryingDefinition& varying, const std::filesystem::path& path) noexcept;

        void writeBgfx(const VaryingDefinition& varying, std::ostream& out, const AttribGroups& disabledGroups = {}) noexcept;
        void writeBgfx(const VaryingDefinition& varying, std::ostream& out, const AttribDefines& defines) noexcept;
        void writeBgfx(const VaryingDefinition& varying, const std::filesystem::path& path) noexcept;

        [[nodiscard]] std::string getBgfxTypeName(bgfx::Attrib::Enum val) noexcept;
        [[nodiscard]] std::string getBgfxVarTypeName(uint8_t num) noexcept;
    };
}

namespace bgfx
{
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