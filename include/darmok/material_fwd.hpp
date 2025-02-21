#pragma once

#include <variant>

namespace darmok
{
    enum class MaterialTextureType
    {
        // PBR
        BaseColor,
        MetallicRoughness,
        Normal,
        Occlusion,
        Emissive,

        // Phong
        Specular,

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