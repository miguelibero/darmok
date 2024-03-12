#include <darmok/color.hpp>

namespace darmok
{
    const Color::Component Color::maxValue = 255;

    Color::Color() noexcept
        : r(0), g(0), b(0), a(0)
    {
    }

    Color::Color(Component r, Component g, Component b, Component a) noexcept
        : r(r), g(g), b(b), a(a)
    {
    }

    Color::Color(const glm::vec4& v) noexcept
    {
        setVector(v);
    }

    void Color::setVector(const glm::vec4& v) noexcept
    {
        r = v.r * Color::maxValue;
        g = v.g * Color::maxValue;
        b = v.b * Color::maxValue;
        a = v.a * Color::maxValue;
    }

    glm::vec4 Color::getVector() const noexcept
    {
        return glm::vec4(
            ((float)r) / Color::maxValue,
            ((float)g) / Color::maxValue,
            ((float)b) / Color::maxValue,
            ((float)a) / Color::maxValue
        );
    }

    const Color::Component* Color::ptr() const noexcept
    {
        return (const Component*)this;
    }

    Color::Component* Color::ptr() noexcept
    {
        return (Component*)this;
    }

    const Color Color::black = { 0, 0, 0, Color::maxValue };
    const Color Color::white = { Color::maxValue, Color::maxValue, Color::maxValue, Color::maxValue };
    const Color Color::red = { Color::maxValue, 0, 0, Color::maxValue };
    const Color Color::green = { 0, Color::maxValue, 0, Color::maxValue };
    const Color Color::blue = { 0, 0, Color::maxValue, Color::maxValue };
}