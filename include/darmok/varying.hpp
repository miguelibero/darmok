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

    class ConstVertexAttributeWrapper
    {
    public:
        using Definition = protobuf::VertexAttribute;
        ConstVertexAttributeWrapper(const Definition& def);

        void addToBgfx(bgfx::VertexLayout& layout) noexcept;
    private:
        const Definition& _def;
    };

    class VertexAttributeWrapper final : public ConstVertexAttributeWrapper
    {
    public:
        VertexAttributeWrapper(Definition& def);
        [[nodiscard]] expected<void, std::string> read(const std::string& key) noexcept;
        [[nodiscard]] expected<void, std::string> read(const std::string& key, const nlohmann::json& json) noexcept;

    private:
        Definition& _def;
    };

    class FragmentAttributeWrapper
    {
    public:
        using Definition = protobuf::FragmentAttribute;
        FragmentAttributeWrapper(Definition& def);

        [[nodiscard]] expected<void, std::string> read(const std::string& key) noexcept;
        [[nodiscard]] expected<void, std::string> read(const std::string& key, const nlohmann::json& json) noexcept;

    private:
        Definition& _def;
    };

    class ConstVertexLayoutWrapper
    {
    public:
        using Definition = protobuf::VertexLayout;
        using Attribute = protobuf::VertexAttribute;
        ConstVertexLayoutWrapper(const Definition& def);

        [[nodiscard]] bgfx::VertexLayout getBgfx(const AttribGroups& disabledGroups = {}) noexcept;
        [[nodiscard]] bgfx::VertexLayout getBgfx(const AttribDefines& defines) noexcept;
    private:
        const Definition& _def;
    };

    class VertexLayoutWrapper final : public ConstVertexLayoutWrapper
    {
    public:
        VertexLayoutWrapper(Definition& def);

        void read(const bgfx::VertexLayout& bgfxLayout) noexcept;
        [[nodiscard]] expected<void, std::string> read(const nlohmann::ordered_json& json) noexcept;
    private:
        Definition& _def;
    };

    class ConstFragmentLayoutWrapper
    {
    public:
        using Definition = protobuf::FragmentLayout;
        ConstFragmentLayoutWrapper(const Definition& def);

        [[nodiscard]] bgfx::Attrib::Enum getUnusedAttrib(const std::vector<bgfx::Attrib::Enum>& used = {}) noexcept;

    private:
        const Definition& _def;
    };

    class FragmentLayoutWrapper final : public ConstFragmentLayoutWrapper
    {
    public:
        FragmentLayoutWrapper(Definition& def);

        [[nodiscard]] expected<void, std::string> read(const nlohmann::ordered_json& json) noexcept;
    private:
        Definition& _def;
    };

    class ConstVaryingDefinitionWrapper
    {
    public:
        using Definition = protobuf::Varying;
        ConstVaryingDefinitionWrapper(const Definition def);

        void writeBgfx(std::ostream& out, const AttribGroups& disabledGroups = {}) noexcept;
        void writeBgfx(std::ostream& out, const AttribDefines& defines) noexcept;
        void writeBgfx(const std::filesystem::path& path) noexcept;
    private:
        const Definition& _def;

        static [[nodiscard]] std::string getBgfxVarTypeName(uint8_t num) noexcept;
        static [[nodiscard]] std::string getBgfxTypeName(bgfx::Attrib::Enum val) noexcept;
    };

    class VaryingDefinitionWrapper final : public ConstVaryingDefinitionWrapper
    {
    public:
        VaryingDefinitionWrapper(Definition def);

        [[nodiscard]] expected<void, std::string> read(const nlohmann::ordered_json& json) noexcept;
        [[nodiscard]] expected<void, std::string> read(const std::filesystem::path& path) noexcept;

    private:
        Definition& _def;
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