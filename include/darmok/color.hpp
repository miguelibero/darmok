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

        Color();
        Color(Component r, Component g, Component b, Component a = maxValue);
        Color(const glm::vec4& v);
        void setVector(const glm::vec4& v);
        glm::vec4 getVector() const;
        const Component* ptr() const;
        Component* ptr();
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