#pragma once

#include <darmok/export.h>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <unordered_map>

namespace darmok
{
    struct DARMOK_EXPORT VertexLayoutUtils final
    {
        static [[nodiscard]] bgfx::Attrib::Enum getBgfxAttrib(const std::string_view name) noexcept;
		static [[nodiscard]] bgfx::AttribType::Enum getBgfxAttribType(const std::string_view name) noexcept;
        static [[nodiscard]] std::string getBgfxAttribName(bgfx::Attrib::Enum val) noexcept;
        static [[nodiscard]] std::string getBgfxAttribTypeName(bgfx::AttribType::Enum val) noexcept;
        static void readFile(const std::string& path, bgfx::VertexLayout& layout) noexcept;
        static void writeFile(const std::string& path, const bgfx::VertexLayout& layout) noexcept;
		static void readJson(const nlohmann::ordered_json& json, bgfx::VertexLayout& layout) noexcept;
        static void writeJson(nlohmann::ordered_json& json, const bgfx::VertexLayout& layout) noexcept;
        static void writeHeader(std::ostream& os, std::string_view varName, const bgfx::VertexLayout& layout) noexcept;
        static void readVaryingDef(std::istream& is, bgfx::VertexLayout& layout) noexcept;
    private:
        static const std::unordered_map<std::string, bgfx::Attrib::Enum>& getVaryingDefAttrs() noexcept;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IVertexLayoutLoader
    {
    public:
        using result_type = bgfx::VertexLayout;
        virtual ~IVertexLayoutLoader() = default;
        virtual [[nodiscard]] result_type operator()(std::string_view name) = 0;
    };

    class IDataLoader;

    class DARMOK_EXPORT BinaryVertexLayoutLoader final : public IVertexLayoutLoader
    {
    public:
        BinaryVertexLayoutLoader(IDataLoader& dataLoader) noexcept;
        [[nodiscard]] bgfx::VertexLayout operator()(std::string_view name) override;
    private:
        IDataLoader& _dataLoader;
    };

    class DARMOK_EXPORT VertexLayoutProcessor final
    {
    public:
        VertexLayoutProcessor(const std::string& inputPath);
        VertexLayoutProcessor& setHeaderVarName(const std::string& name) noexcept;
        std::string to_string() const noexcept;
        const bgfx::VertexLayout& getVertexLayout() const noexcept;
        void writeFile(const std::string& outputPath);
    private:
        bgfx::VertexLayout _layout;
        std::string _inputPath;
        std::string _headerVarName;
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
DARMOK_EXPORT std::ostream& operator<<(std::ostream& out, const darmok::VertexLayoutProcessor& process) noexcept;
