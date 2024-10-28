#pragma once

#include <variant>

namespace darmok
{
    enum class MaterialTextureType
    {
        BaseColor,
        Specular,
        MetallicRoughness,
        Normal,
        Occlusion,
        Emissive,

        Count
    };

    enum class MaterialPrimitiveType
    {
        Triangle,
        Line,

        Count
    };

    enum class OpacityType
    {
        Opaque,
        Mask,
        Transparent,

        Count
    };
}