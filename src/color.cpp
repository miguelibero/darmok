#include <darmok/color.hpp>

namespace darmok
{
    const Color::value_type Colors::maxValue = 255;

    const Color3 Colors::black3 = { 0, 0, 0 };
    const Color3 Colors::white3 = { maxValue, maxValue, maxValue };
    const Color3 Colors::red3 = { maxValue, 0, 0 };
    const Color3 Colors::green3 = { 0, maxValue, 0 };
    const Color3 Colors::blue3 = { 0, 0, maxValue };
    const Color3 Colors::yellow3 = { maxValue, maxValue, 0 };
    const Color3 Colors::cyan3 = { 0, maxValue, maxValue };
    const Color3 Colors::magenta3 = { maxValue, 0, maxValue };

    const Color Colors::black = Color(Colors::black3, maxValue);
    const Color Colors::white = Color(Colors::white3, maxValue);
    const Color Colors::red = Color(Colors::red3, maxValue);
    const Color Colors::green = Color(Colors::green3, maxValue);
    const Color Colors::blue = Color(Colors::blue3, maxValue);
    const Color Colors::yellow = Color(Colors::yellow3, maxValue);
    const Color Colors::cyan = Color(Colors::cyan3, maxValue);
    const Color Colors::magenta = Color(Colors::magenta3, maxValue);

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