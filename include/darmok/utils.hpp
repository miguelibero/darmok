#pragma once

#include <bx/error.h>
#include <bgfx/bgfx.h>
#include <string_view>

namespace darmok
{
    template <typename E>
    [[nodiscard]] constexpr auto to_underlying(E e) noexcept
    {
        return static_cast<std::underlying_type_t<E>>(e);
    }

    std::string strToLower(std::string_view sv) noexcept;
    void checkError(bx::Error& err);    

    struct Utf8Char final
    {
        uint32_t data;
        uint8_t len;

        Utf8Char(uint32_t data = 0, uint8_t len = 0) noexcept;
        [[nodiscard]] static Utf8Char encode(uint32_t scancode) noexcept;
        std::string_view string() const noexcept;
    };
}