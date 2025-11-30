#pragma once

#include <darmok/export.h>

#include <type_traits>
#include <vector>
#include <string>
#include <variant>
#include <filesystem>
#include <optional>

#include <bx/error.h>
#include <entt/entt.hpp>

namespace darmok
{
    template <typename E>
    [[nodiscard]] constexpr auto toUnderlying(E e) noexcept
    {
        return static_cast<std::underlying_type_t<E>>(e);
    }

    [[nodiscard]] std::optional<std::string> checkError(bx::Error& err) noexcept;

    class DARMOK_EXPORT RandomGenerator final
    {
    public:
        RandomGenerator(uint32_t seed) noexcept;

        template<typename T>
        T operator()() noexcept
        {
            _seed = hash(_seed);
            return (T)_seed / (T)UINT32_MAX;
        }

    private:
        uint32_t _seed;

        static uint32_t hash(uint32_t input) noexcept;
    };

    namespace DARMOK_EXPORT Exec
    {
        struct Result final
        {
            std::string out;
            std::string err;
            int returnCode;
        };

        using Arg = std::variant<const char*, std::string, std::filesystem::path>;
        Result run(const std::vector<Arg>& args, const std::filesystem::path& cwd = "");
        std::string argsToString(const std::vector<Arg>& args);
    };

    std::filesystem::path getTempPath(std::string_view prefix = "") noexcept;

    inline void hashCombine(std::size_t& seed) { }

    template <typename T, typename... Rest>
    inline void hashCombine(std::size_t& seed, const T& v, Rest... rest)
    {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        hashCombine(seed, rest...);
    }

    inline entt::id_type idTypeCombine(entt::id_type seed, entt::id_type other)
    {
        seed ^= other + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }

    entt::id_type getPtrId(const void* ptr) noexcept;


}
