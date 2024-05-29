#include <darmok/color.hpp>

namespace darmok
{
    const Color::value_type Colors::maxValue = 255;

    const Color3& Colors::black3() noexcept
    {
        const static Color3 v{ 0, 0, 0 };
        return v;
    }

    const Color3& Colors::grey3() noexcept
    {
        const static Color3 v{ maxValue / 2, maxValue / 2, maxValue / 2 };
        return v;
    }
        
    const Color3& Colors::white3() noexcept
    {
        const static Color3 v{ maxValue, maxValue, maxValue };
        return v;
    }

    const Color3& Colors::red3() noexcept
    {
        const static Color3 v{ maxValue, 0, 0 };
        return v;
    }

    const Color3& Colors::green3() noexcept
    {
        const static Color3 v{ 0, maxValue, 0 };
        return v;
    }

    const Color3& Colors::blue3() noexcept
    {
        const static Color3 v{ 0, 0, maxValue };
        return v;
    }

    const Color3& Colors::yellow3() noexcept
    {
        const static Color3 v{ maxValue, maxValue, 0 };
        return v;
    }

    const Color3& Colors::cyan3() noexcept
    {
        const static Color3 v{ 0, maxValue, maxValue };
        return v;
    }

    const Color3& Colors::magenta3() noexcept
    {
        const static Color3 v{ maxValue, 0, maxValue };
        return v;
    }

    const Color& Colors::black() noexcept
    {
        const static Color v(black3(), maxValue);
        return v;
    }

    const Color& Colors::grey() noexcept
    {
        const static Color v(grey3(), maxValue);
        return v;
    }

    const Color& Colors::white() noexcept
    {
        const static Color v(white3(), maxValue);
        return v;
    }

    const Color& Colors::red() noexcept
    {
        const static Color v(red3(), maxValue);
        return v;
    }

    const Color& Colors::green() noexcept
    {
        const static Color v(green3(), maxValue);
        return v;
    }

    const Color& Colors::blue() noexcept
    {
        const static Color v(blue3(), maxValue);
        return v;
    }

    const Color& Colors::yellow() noexcept
    {
        const static Color v(yellow3(), maxValue);
        return v;
    }

    const Color& Colors::cyan() noexcept
    {
        const static Color v(cyan3(), maxValue);
        return v;
    }

    const Color& Colors::magenta() noexcept
    {
        const static Color v(magenta3(), maxValue);
        return v;
    }


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
        return uint32_t(color.r) << 24 | uint32_t(color.g) << 16 | uint32_t(color.b) << 8 | uint32_t(color.a);
    }

    uint32_t Colors::toNumber(const Color3& color) noexcept
    {
        return uint32_t(color.r) << 24 | uint32_t(color.g) << 16 | uint32_t(color.b) << 8 | 0xff;
    }

    uint32_t Colors::toReverseNumber(const Color& color) noexcept
    {
        return uint32_t(color.a) << 24 | uint32_t(color.b) << 16 | uint32_t(color.g) << 8 | uint32_t(color.r);
    }

    uint32_t Colors::toReverseNumber(const Color3& color) noexcept
    {
        return 0xff << 24 | uint32_t(color.b) << 16 | uint32_t(color.g) << 8 | uint32_t(color.r);
    }
}