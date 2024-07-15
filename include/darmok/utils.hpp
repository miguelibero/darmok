#pragma once

#include <bx/error.h>
#include <type_traits>
#include <vector>
#include <string>
#include <type_traits>
#include <filesystem>

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

    std::filesystem::path getTempPath() noexcept;

    inline void hash_combine(std::size_t& seed) { }

    template <typename T, typename... Rest>
    inline void hash_combine(std::size_t& seed, const T& v, Rest... rest)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        hash_combine(seed, rest...);
    }
}