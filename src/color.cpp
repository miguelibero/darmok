#include <darmok/color.hpp>

namespace darmok
{
    const Color::value_type& Colors::getMaxValue() noexcept
    {
        static const Color::value_type maxValue = 255;
        return maxValue;
    }

    const Color3& Colors::black3() noexcept
    {
        const static Color3 v{ 0, 0, 0 };
        return v;
    }

    const Color3& Colors::grey3() noexcept
    {
        const static Color3 v{ getMaxValue() / 2, getMaxValue() / 2, getMaxValue() / 2 };
        return v;
    }
        
    const Color3& Colors::white3() noexcept
    {
        const static Color3 v{ getMaxValue(), getMaxValue(), getMaxValue() };
        return v;
    }

    const Color3& Colors::red3() noexcept
    {
        const static Color3 v{ getMaxValue(), 0, 0 };
        return v;
    }

    const Color3& Colors::green3() noexcept
    {
        const static Color3 v{ 0, getMaxValue(), 0 };
        return v;
    }

    const Color3& Colors::blue3() noexcept
    {
        const static Color3 v{ 0, 0, getMaxValue() };
        return v;
    }

    const Color3& Colors::yellow3() noexcept
    {
        const static Color3 v{ getMaxValue(), getMaxValue(), 0 };
        return v;
    }

    const Color3& Colors::cyan3() noexcept
    {
        const static Color3 v{ 0, getMaxValue(), getMaxValue() };
        return v;
    }

    const Color3& Colors::magenta3() noexcept
    {
        const static Color3 v{ getMaxValue(), 0, getMaxValue() };
        return v;
    }

    const Color& Colors::black() noexcept
    {
        const static Color v{ black3(), getMaxValue() };
        return v;
    }

    const Color& Colors::grey() noexcept
    {
        const static Color v{ grey3(), getMaxValue() };
        return v;
    }

    const Color& Colors::white() noexcept
    {
        const static Color v{ white3(), getMaxValue() };
        return v;
    }

    const Color& Colors::red() noexcept
    {
        const static Color v{ red3(), getMaxValue() };
        return v;
    }

    const Color& Colors::green() noexcept
    {
        const static Color v{ green3(), getMaxValue() };
        return v;
    }

    const Color& Colors::blue() noexcept
    {
        const static Color v{ blue3(), getMaxValue() };
        return v;
    }

    const Color& Colors::yellow() noexcept
    {
        const static Color v{ yellow3(), getMaxValue() };
        return v;
    }

    const Color& Colors::cyan() noexcept
    {
        const static Color v{ cyan3(), getMaxValue() };
        return v;
    }

    const Color& Colors::magenta() noexcept
    {
        const static Color v{ magenta3(), getMaxValue() };
        return v;
    }

    const Color& Colors::debug(uint8_t position) noexcept
    {
        position %= 6;
        switch (position)
        {
        case 0:
            return magenta();
        case 1:
            return cyan();
        case 2:
            return red();
        case 3:
            return green();
        case 4:
            return blue();
        case 5:
            return yellow();
        }
        return black();
    }

    const Color3& Colors::debug3(uint8_t position) noexcept
    {
        position %= 6;
        switch (position)
        {
        case 0:
            return magenta3();
        case 1:
            return cyan3();
        case 2:
            return red3();
        case 3:
            return green3();
        case 4:
            return blue3();
        case 5:
            return yellow3();
        }
        return black3();
    }

    glm::vec4 Colors::normalize(const Color& color) noexcept
    {
        return glm::vec4{
            static_cast<float>(color.r) / getMaxValue(),
            static_cast<float>(color.g) / getMaxValue(),
            static_cast<float>(color.b) / getMaxValue(),
            static_cast<float>(color.a) / getMaxValue()
        };
    }

    glm::vec3 Colors::normalize(const Color3& color) noexcept
    {
        return glm::vec3{
            static_cast<float>(color.r) / getMaxValue(),
            static_cast<float>(color.g) / getMaxValue(),
            static_cast<float>(color.b) / getMaxValue()
        };
    }

    Color Colors::denormalize(const glm::vec4& v) noexcept
    {
        return Color{
            v.r * getMaxValue(),
            v.g * getMaxValue(),
            v.b * getMaxValue(),
            v.a * getMaxValue()
        };
    }

    Color3 Colors::denormalize(const glm::vec3& v) noexcept
    {
        return Color3{
            v.r * getMaxValue(),
            v.g * getMaxValue(),
            v.b * getMaxValue()
        };
    }


    // TODO: check that this works as expected with bimg::solid, I think alpha is not working

    Color Colors::fromNumber(uint32_t v) noexcept
    {
        return Color{ v >> 24, v >> 16, v >> 8, v };
    }

    uint32_t Colors::toNumber(const Color& color) noexcept
    {
        return static_cast<uint32_t>(color.r) << 24
            | static_cast<uint32_t>(color.g) << 16
            | static_cast<uint32_t>(color.b) << 8
            | static_cast<uint32_t>(color.a);
    }

    uint32_t Colors::toNumber(const Color3& color) noexcept
    {
        return static_cast<uint32_t>(color.r) << 24
            | static_cast<uint32_t>(color.g) << 16
            | static_cast<uint32_t>(color.b) << 8 | 0xff;
    }

    uint32_t Colors::toReverseNumber(const Color& color) noexcept
    {
        return static_cast<uint32_t>(color.a) << 24
            | static_cast<uint32_t>(color.b) << 16
            | static_cast<uint32_t>(color.g) << 8
            | static_cast<uint32_t>(color.r);
    }

    uint32_t Colors::toReverseNumber(const Color3& color) noexcept
    {
        return 0xff << 24
            | static_cast<uint32_t>(color.b) << 16
            | static_cast<uint32_t>(color.g) << 8
            | static_cast<uint32_t>(color.r);
    }

    Color Colors::multiply(const Color& a, const Color& b) noexcept
    {
        return glm::vec4{ a } * normalize(b);
    }

    Color3 Colors::multiply(const Color3& a, const Color3& b) noexcept
    {
        return glm::vec3{ a } * normalize(b);
    }

    Color Colors::divide(const Color& a, const Color& b) noexcept
    {
        return glm::vec4{ a } / normalize(b);
    }

    Color3 Colors::divide(const Color3& a, const Color3& b) noexcept
    {
        return glm::vec3{ a } / normalize(b);
    }

}