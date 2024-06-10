#pragma once

#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <unordered_map>

namespace darmok
{
    struct VertexLayoutUtils final
    {
        static bgfx::Attrib::Enum getBgfxAttrib(const std::string_view name) noexcept;
		static bgfx::AttribType::Enum getBgfxAttribType(const std::string_view name) noexcept;
        static std::string getBgfxAttribName(bgfx::Attrib::Enum val) noexcept;
        static std::string getBgfxAttribTypeName(bgfx::AttribType::Enum val) noexcept;
        static void readFile(const std::string& path, bgfx::VertexLayout& layout) noexcept;
        static void writeFile(const std::string& path, const bgfx::VertexLayout& layout) noexcept;
		static void readJson(const nlohmann::ordered_json& json, bgfx::VertexLayout& layout) noexcept;
        static void writeJson(nlohmann::ordered_json& json, const bgfx::VertexLayout& layout) noexcept;
        static void writeHeader(std::ostream& os, std::string_view varName, const bgfx::VertexLayout& layout) noexcept;
        static void readVaryingDef(std::istream& is, bgfx::VertexLayout& layout) noexcept;
    private:
        static std::unordered_map<std::string, bgfx::Attrib::Enum> _varyingDefAttrs;
    };

    class BX_NO_VTABLE IVertexLayoutLoader
    {
    public:
        using result_type = bgfx::VertexLayout;
        DLLEXPORT virtual ~IVertexLayoutLoader() = default;
        DLLEXPORT virtual result_type operator()(std::string_view name) = 0;
    };

    class IDataLoader;

    class BinaryVertexLayoutLoader final : public IVertexLayoutLoader
    {
    public:
        BinaryVertexLayoutLoader(IDataLoader& dataLoader) noexcept;
        bgfx::VertexLayout operator()(std::string_view name) override;
    private:
        IDataLoader& _dataLoader;
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

std::string to_string(const bgfx::VertexLayout& layout) noexcept;
std::ostream& operator<<(std::ostream& out, const bgfx::VertexLayout& layout);