#pragma once

#include <expected>
#include <string>

namespace darmok
{
    template<class T, class E>
    using expected = std::expected<T, E>;

    template<class E>
    using unexpected = std::unexpected<E>;

    inline unexpected<std::string> prefixError(std::string_view prefix, const std::string& error) noexcept
    {
        return unexpected<std::string>{std::string(prefix) + ": " + error};
    }
}

#define DARMOK_TRY_VALUE_PREFIX(value, failable, prefix)            \
    do {                                                            \
        auto result = (failable);                                   \
        if (!result)                                                \
            return darmok::prefixError(prefix, result.error());     \
        value = std::move(*result);                                 \
    } while (false)

#define DARMOK_TRY_VALUE(value, failable)                           \
    do {                                                            \
        auto result = (failable);                                   \
        if (!result)                                                \
            return darmok::unexpected{ std::move(result).error() }; \
        value = std::move(*result);                                 \
    } while (false)

#define DARMOK_TRY_PREFIX(failable, prefix)                         \
    do {                                                            \
        auto result = (failable);                                   \
        if (!result)                                                \
            return darmok::prefixError(prefix, result.error());     \
    } while (false)

#define DARMOK_TRY(failable)                                        \
    do {                                                            \
        auto result = (failable);                                   \
        if (!result)                                                \
            return darmok::unexpected{ std::move(result).error() }; \
    } while (false)
