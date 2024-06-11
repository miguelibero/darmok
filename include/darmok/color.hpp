#pragma once

#include <darmok/export.h>
#include <darmok/color_fwd.hpp>

namespace darmok
{
    struct DARMOK_EXPORT Colors final
    {
        static [[nodiscard]] const Color::value_type& getMaxValue() noexcept;

        static const Color& black() noexcept;
        static const Color& grey() noexcept;
        static const Color& white() noexcept;
        static const Color& red() noexcept;
        static const Color& green() noexcept;
        static const Color& blue() noexcept;
        static const Color& yellow() noexcept;
        static const Color& cyan() noexcept;
        static const Color& magenta() noexcept;

        static const Color3& black3() noexcept;
        static const Color3& grey3() noexcept;
        static const Color3& white3() noexcept;
        static const Color3& red3() noexcept;
        static const Color3& green3() noexcept;
        static const Color3& blue3() noexcept;
        static const Color3& yellow3() noexcept;
        static const Color3& cyan3() noexcept;
        static const Color3& magenta3() noexcept;

        static glm::vec4 normalize(const Color& color) noexcept;
        static glm::vec3 normalize(const Color3& color) noexcept;
        static Color fromNumber(uint32_t color) noexcept;
        static uint32_t toNumber(const Color& color) noexcept;
        static uint32_t toNumber(const Color3& color) noexcept;
        static uint32_t toReverseNumber(const Color& color) noexcept;
        static uint32_t toReverseNumber(const Color3& color) noexcept;
    };
}

namespace std
{
    template<typename T>
    struct hash;
}

template<> struct std::hash<darmok::Color>
{
    std::size_t operator()(darmok::Color const& key) const noexcept
    {
        return darmok::Colors::toNumber(key);
    }
};

template<> struct std::hash<darmok::Color3>
{
    std::size_t operator()(darmok::Color3 const& key) const noexcept
    {
        return darmok::Colors::toNumber(key);
    }
};