#pragma once

#include <bx/error.h>
#include <type_traits>
#include <iostream>
#include <vector>

namespace darmok
{
    template <typename E>
    [[nodiscard]] constexpr auto to_underlying(E e) noexcept
    {
        return static_cast<std::underlying_type_t<E>>(e);
    }

    void checkError(bx::Error& err);

    void copyStream(std::istream& input, std::ostream& output, size_t bufferSize = 4096);

    std::string escapeArgument(const std::string& arg);

    struct ExecResult final
    {
        std::string output;
        int returnCode;
    };

    ExecResult exec(const std::vector<std::string>& args);
}