#pragma once

#include <glm/glm.hpp>

namespace darmok
{
    struct Color
    {
        typedef uint8_t Component;
        static const Component maxValue;

        Component r;
        Component g;
        Component b;
        Component a;

        Color() noexcept;
        Color(Component r, Component g, Component b, Component a = maxValue) noexcept;
        Color(const glm::vec4& v) noexcept;
        
        void setVector(const glm::vec4& v) noexcept;

        [[nodiscard]] glm::vec4 getVector() const noexcept;
        [[nodiscard]] const Component* ptr() const noexcept;
        [[nodiscard]] Component* ptr() noexcept;
    };

    struct Colors
    {
        static const Color black;
        static const Color white;
        static const Color red;
        static const Color green;
        static const Color blue;
    };
}