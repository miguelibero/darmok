#pragma once

#include <glm/glm.hpp>

namespace darmok
{
    using Color = glm::u8vec4;
    using Color3 = glm::u8vec3;

    struct Colors final
    {
        static const Color::value_type maxValue;
        static const Color black;
        static const Color white;
        static const Color red;
        static const Color green;
        static const Color blue;

        static glm::vec4 normalize(const Color& color) noexcept;
        static glm::vec3 normalize(const Color3& color) noexcept;
        static Color fromNumber(uint32_t color) noexcept;
        static uint32_t toNumber(const Color& color) noexcept;
        static uint32_t toNumber(const Color3& color) noexcept;
    };
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