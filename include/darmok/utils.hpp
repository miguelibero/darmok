#pragma once

#include <type_traits>
#include <bx/error.h>
#include <bgfx/bgfx.h>
#include <vector>

namespace darmok
{
    template <typename E>
    constexpr auto to_underlying(E e) noexcept
    {
        return static_cast<std::underlying_type_t<E>>(e);
    }

    template<typename T>
    static const bgfx::Memory* makeVectorRef(const std::vector<T>& v)
    {
        if (v.empty())
        {
            return nullptr;
        }
        return bgfx::makeRef(&v.front(), v.size() * sizeof(T));
    }

    void checkError(bx::Error& err);
}