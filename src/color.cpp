#include <darmok/color.hpp>

namespace darmok
{
    const uint8_t Colors::maxValue = 255;
    const Color Colors::black = { 0, 0, 0, maxValue };
    const Color Colors::white = { maxValue, maxValue, maxValue, maxValue };
    const Color Colors::red = { maxValue, 0, 0, maxValue };
    const Color Colors::green = { 0, maxValue, 0, maxValue };
    const Color Colors::blue = { 0, 0, maxValue, maxValue };
}