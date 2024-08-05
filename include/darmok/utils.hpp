#pragma once

#include <bx/error.h>
#include <type_traits>
#include <vector>
#include <string>
#include <variant>
#include <type_traits>
#include <filesystem>
#include <entt/entt.hpp>

namespace darmok
{
    template <typename E>
    [[nodiscard]] constexpr auto to_underlying(E e) noexcept
    {
        return static_cast<std::underlying_type_t<E>>(e);
    }

    void checkError(bx::Error& err);

    struct Exec final
    {
        struct Result final
        {
            std::string output;
            int returnCode;
        };

        using Arg = std::variant<const char*, std::string, std::filesystem::path>;
        static Result run(const std::vector<Arg>& args);
    };

    std::filesystem::path getTempPath(std::string_view suffix = "") noexcept;

    inline void hashCombine(std::size_t& seed) { }

    template <typename T, typename... Rest>
    inline void hashCombine(std::size_t& seed, const T& v, Rest... rest)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        hashCombine(seed, rest...);
    }

    inline entt::id_type idTypeCombine(entt::id_type seed, entt::id_type other)
    {
        seed ^= other + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }

    entt::id_type randomIdType() noexcept;
}