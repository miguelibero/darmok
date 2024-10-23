#pragma once

#include <darmok/export.h>
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
    [[nodiscard]] constexpr auto toUnderlying(E e) noexcept
    {
        return static_cast<std::underlying_type_t<E>>(e);
    }

    void checkError(bx::Error& err);

    class DARMOK_EXPORT Random final
    {
    public:
        Random(uint32_t seed) noexcept;

        template<typename T>
        T generate() noexcept
        {
            _seed = hash(_seed);
            return (T)_seed / (T)UINT32_MAX;
        }

    private:
        uint32_t _seed;

        static uint32_t hash(uint32_t input) noexcept;
    };

    struct DARMOK_EXPORT Exec final
    {
        struct Result final
        {
            std::string out;
            std::string err;
            int returnCode;
        };

        using Arg = std::variant<const char*, std::string, std::filesystem::path>;
        static Result run(const std::vector<Arg>& args, const std::filesystem::path& cwd = "");
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

    struct DARMOK_EXPORT TypeFilter final
    {
        using Container = std::unordered_set<entt::id_type>;

        template<typename T>
        TypeFilter& include() noexcept
        {
            return include(entt::type_hash<T>::value());
        }

        TypeFilter& include(entt::id_type idType) noexcept;

        template<typename T>
        TypeFilter& exclude() noexcept
        {
            return exclude(entt::type_hash<T>::value());
        }

        TypeFilter& exclude(entt::id_type idType) noexcept;

        const Container& getIncludes() const noexcept;
        const Container& getExcludes() const noexcept;
        std::string toString() const noexcept;

        bool empty() const noexcept;
        bool matches(entt::id_type) const noexcept;
        bool operator()(entt::id_type) const noexcept;

        bool operator==(const TypeFilter& other) const noexcept;
        bool operator!=(const TypeFilter& other) const noexcept;

        TypeFilter operator+(const TypeFilter& other) const noexcept;
        TypeFilter& operator+=(const TypeFilter& other) noexcept;

    private:
        Container _includes;
        Container _excludes;
    };
}