#pragma once

#include <darmok/export.h>
#include <darmok/color_fwd.hpp>

namespace darmok
{
    struct DARMOK_EXPORT Colors final
    {
        [[nodiscard]] static const Color::value_type& getMaxValue() noexcept;

        static const Color& debug(uint8_t position) noexcept;

        static const Color& black() noexcept;
        static const Color& grey() noexcept;
        static const Color& white() noexcept;
        static const Color& red() noexcept;
        static const Color& green() noexcept;
        static const Color& blue() noexcept;
        static const Color& yellow() noexcept;
        static const Color& cyan() noexcept;
        static const Color& magenta() noexcept;

        static const Color3& debug3(uint8_t position) noexcept;

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
        static Color denormalize(const glm::vec4& v) noexcept;
        static Color3 denormalize(const glm::vec3& v) noexcept;

        static Color fromNumber(uint32_t color) noexcept;
        static uint32_t toNumber(const Color& color) noexcept;
        static uint32_t toNumber(const Color3& color) noexcept;
        static uint32_t toReverseNumber(const Color& color) noexcept;
        static uint32_t toReverseNumber(const Color3& color) noexcept;

        static Color multiply(const Color& a, const Color& b) noexcept;
        static Color3 multiply(const Color3& a, const Color3& b) noexcept;
        static Color divide(const Color& a, const Color& b) noexcept;
        static Color3 divide(const Color3& a, const Color3& b) noexcept;
    };
}

namespace std
{
    template<> struct hash<darmok::Color>
    {
        std::size_t operator()(const darmok::Color& key) const noexcept
        {
            return darmok::Colors::toNumber(key);
        }
    };

    template<> struct hash<darmok::Color3>
    {
        std::size_t operator()(const darmok::Color3& key) const noexcept
        {
            return darmok::Colors::toNumber(key);
        }
    };
}
