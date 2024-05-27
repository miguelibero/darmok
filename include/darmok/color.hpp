#pragma once

#include <darmok/color_fwd.hpp>

namespace darmok
{
    struct Colors final
    {
        static const Color::value_type maxValue;

        DLLEXPORT static const Color& black() noexcept;
        DLLEXPORT static const Color& white() noexcept;
        DLLEXPORT static const Color& red() noexcept;
        DLLEXPORT static const Color& green() noexcept;
        DLLEXPORT static const Color& blue() noexcept;
        DLLEXPORT static const Color& yellow() noexcept;
        DLLEXPORT static const Color& cyan() noexcept;
        DLLEXPORT static const Color& magenta() noexcept;

        DLLEXPORT static const Color3& black3() noexcept;
        DLLEXPORT static const Color3& white3() noexcept;
        DLLEXPORT static const Color3& red3() noexcept;
        DLLEXPORT static const Color3& green3() noexcept;
        DLLEXPORT static const Color3& blue3() noexcept;
        DLLEXPORT static const Color3& yellow3() noexcept;
        DLLEXPORT static const Color3& cyan3() noexcept;
        DLLEXPORT static const Color3& magenta3() noexcept;

        DLLEXPORT static glm::vec4 normalize(const Color& color) noexcept;
        DLLEXPORT static glm::vec3 normalize(const Color3& color) noexcept;
        DLLEXPORT static Color fromNumber(uint32_t color) noexcept;
        DLLEXPORT static uint32_t toNumber(const Color& color) noexcept;
        DLLEXPORT static uint32_t toNumber(const Color3& color) noexcept;
        DLLEXPORT static uint32_t toReverseNumber(const Color& color) noexcept;
        DLLEXPORT static uint32_t toReverseNumber(const Color3& color) noexcept;
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