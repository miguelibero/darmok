#pragma once

#include <type_traits>

namespace darmok
{
    template<typename Dst, typename Src, typename = void>
    struct Converter
    {
        static Dst run(const Src& src)
            requires (
        std::is_convertible_v<Src, Dst> ||
            std::constructible_from<Dst, Src>
            )
        {
            if constexpr (std::is_convertible_v<Src, Dst>)
            {
                return static_cast<Dst>(src);
            }
            else
            {
                return Dst(src);
            }
        }
    };

    template<typename Dst, typename Src>
    Dst convert(const Src& src)
    {
        return Converter<Dst, Src>::run(src);
    }

    template<typename Dst, typename Src>
    concept IsConvertible =
        requires(const Src& src) {
            { convert<Dst, Src>(src) } -> std::convertible_to<Dst>;
    };

    template<typename A, typename B>
    concept IsMutuallyConvertible = IsConvertible<A, B> && IsConvertible<B, A>;

}