#pragma once

#include <bx/error.h>
#include <type_traits>
#include <vector>
#include <string>

namespace darmok
{
    template <typename E>
    [[nodiscard]] constexpr auto to_underlying(E e) noexcept
    {
        return static_cast<std::underlying_type_t<E>>(e);
    }

    void checkError(bx::Error& err);

    struct ExecResult final
    {
        std::string output;
        int returnCode;
    };

    ExecResult exec(const std::vector<std::string>& args);    
}