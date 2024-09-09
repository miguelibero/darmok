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
        Emissive
    };

    enum class MaterialPrimitiveType
    {
        Triangle,
        Line
    };

    enum class MaterialOpacity
    {
        Opaque,
        Cutout,
        Transparent
    };
}