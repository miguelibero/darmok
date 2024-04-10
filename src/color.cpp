#include <darmok/color.hpp>

namespace darmok
{
    const Color::value_type Colors::maxValue = 255;

    const Color Colors::black = { 0, 0, 0, maxValue };
    const Color Colors::white = { maxValue, maxValue, maxValue, maxValue };
    const Color Colors::red = { maxValue, 0, 0, maxValue };
    const Color Colors::green = { 0, maxValue, 0, maxValue };
    const Color Colors::blue = { 0, 0, maxValue, maxValue };
    const Color Colors::yellow = { maxValue, maxValue, 0, maxValue };
    const Color Colors::cyan = { 0, maxValue, maxValue, maxValue };
    const Color Colors::magenta = { maxValue, 0, maxValue, maxValue };

    glm::vec4 Colors::normalize(const Color& color) noexcept
    {
        return glm::vec4(
            (float)color.r / maxValue,
            (float)color.g / maxValue,
            (float)color.b / maxValue,
            (float)color.a / maxValue
            );
    }

    glm::vec3 Colors::normalize(const Color3& color) noexcept
    {
        return glm::vec3(
            (float)color.r / maxValue,
            (float)color.g / maxValue,
            (float)color.b / maxValue
        );
    }

    Color Colors::fromNumber(uint32_t v) noexcept
    {
        return Color(v >> 24, v >> 16, v >> 8, v);
    }

    uint32_t Colors::toNumber(const Color& color) noexcept
    {
        return uint32_t(color.a) << 24 | uint32_t(color.b) << 16 | uint32_t(color.g) << 8 | uint32_t(color.r);
    }

    uint32_t Colors::toNumber(const Color3& color) noexcept
    {
        return 0xff << 24 | uint32_t(color.b) << 16 | uint32_t(color.g) << 8 | uint32_t(color.r);
    }
}