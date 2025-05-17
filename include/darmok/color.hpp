#pragma once

#include <darmok/export.h>
#include <darmok/color_fwd.hpp>

#include <functional>

namespace darmok
{
    namespace Colors
    {
        [[nodiscard]] const Color::value_type& getMaxValue() noexcept;

        const Color& debug(uint8_t position) noexcept;

        const Color& black() noexcept;
        const Color& grey() noexcept;
        const Color& white() noexcept;
        const Color& red() noexcept;
        const Color& green() noexcept;
        const Color& blue() noexcept;
        const Color& yellow() noexcept;
        const Color& cyan() noexcept;
        const Color& magenta() noexcept;

        const Color3& debug3(uint8_t position) noexcept;

        const Color3& black3() noexcept;
        const Color3& grey3() noexcept;
        const Color3& white3() noexcept;
        const Color3& red3() noexcept;
        const Color3& green3() noexcept;
        const Color3& blue3() noexcept;
        const Color3& yellow3() noexcept;
        const Color3& cyan3() noexcept;
        const Color3& magenta3() noexcept;

        glm::vec4 normalize(const Color& color) noexcept;
        glm::vec3 normalize(const Color3& color) noexcept;
        Color denormalize(const glm::vec4& v) noexcept;
        Color3 denormalize(const glm::vec3& v) noexcept;

        Color fromNumber(uint32_t color) noexcept;
        uint32_t toNumber(const Color& color) noexcept;
        uint32_t toNumber(const Color3& color) noexcept;
        uint32_t toReverseNumber(const Color& color) noexcept;
        uint32_t toReverseNumber(const Color3& color) noexcept;

        Color multiply(const Color& a, const Color& b) noexcept;
        Color3 multiply(const Color3& a, const Color3& b) noexcept;
        Color divide(const Color& a, const Color& b) noexcept;
        Color3 divide(const Color3& a, const Color3& b) noexcept;
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
