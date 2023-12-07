#pragma once

#include <type_traits>
#include <bx/error.h>

namespace darmok
{
    template <typename E>
    constexpr auto to_underlying(E e) noexcept
    {
        return static_cast<std::underlying_type_t<E>>(e);
    }

    void checkError(bx::Error& err);
}