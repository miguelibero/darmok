#pragma once

#include <tl/expected.hpp>

namespace darmok
{
    template<class T, class E>
    using expected = tl::expected<T, E>;

    template<class E>
    using unexpected = tl::unexpected<E>;
}

#define DARMOK_TRY_VALUE_PREFIX(value, failable, prefix)                        \
    do {                                                                        \
        auto result = (failable);                                               \
        if (!result)                                                            \
            return darmok::unexpected{ std::string{prefix} + result.error() };  \
        value = std::move(*result);                                             \
    } while (false)

#define DARMOK_TRY_VALUE(value, failable)                           \
    do {                                                            \
        auto result = (failable);                                   \
        if (!result)                                                \
            return darmok::unexpected{ std::move(result).error() }; \
        value = std::move(*result);                                 \
    } while (false)

#define DARMOK_TRY_PREFIX(failable, prefix)                                     \
    do {                                                                        \
        auto result = (failable);                                               \
        if (!result)                                                            \
            return darmok::unexpected{ std::string{prefix} + result.error() };  \
    } while (false)

#define DARMOK_TRY(failable)                                        \
    do {                                                            \
        auto result = (failable);                                   \
        if (!result)                                                \
            return darmok::unexpected{ std::move(result).error() }; \
    } while (false)