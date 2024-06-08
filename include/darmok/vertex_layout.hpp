#pragma once

#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>

namespace darmok
{
    struct VertexLayoutUtils final
    {
        static bgfx::Attrib::Enum getBgfxAttrib(const std::string_view name) noexcept;
		static bgfx::AttribType::Enum getBgfxAttribType(const std::string_view name) noexcept;
        static std::string getBgfxAttribName(bgfx::Attrib::Enum val) noexcept;
        static std::string getBgfxAttribTypeName(bgfx::AttribType::Enum val) noexcept;
		static void readJson(const nlohmann::ordered_json& json, bgfx::VertexLayout& layout) noexcept;
        static void writeJson(nlohmann::ordered_json& json, const bgfx::VertexLayout& layout) noexcept;
        static void readVaryingDef(std::string_view content, bgfx::VertexLayout& layout) noexcept;
    };

    class BX_NO_VTABLE IVertexLayoutLoader
    {
    public:
        using result_type = bgfx::VertexLayout;
        DLLEXPORT virtual ~IVertexLayoutLoader() = default;
        DLLEXPORT virtual result_type operator()(std::string_view name) = 0;
    };
}

namespace bgfx
{
    // TODO: check if bgfx::read/write VertexLayout can be accessed
    // https://github.com/bkaradzic/bgfx/blob/master/src/vertexlayout.h#L39
    // instead of creating custom serialization
    template<class Archive>
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