#pragma once

namespace darmok
{
    enum class MaterialTextureType
    {
        Base,
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
}