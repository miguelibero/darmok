#pragma once

#include <glm/glm.hpp>

namespace darmok
{
    struct Color
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };

    struct Colors
    {
        static const uint8_t maxValue;
        static const Color black;
        static const Color white;
        static const Color red;
        static const Color green;
        static const Color blue;
    };
}