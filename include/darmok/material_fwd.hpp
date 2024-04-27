#pragma once

namespace darmok
{
    enum class MaterialTextureType
    {
        Unknown,
        Diffuse,
        Specular,
        Normal,
        Count,
    };

    enum class MaterialColorType
    {
        Diffuse,
        Specular,
        Ambient,
        Emissive,
        Transparent,
        Reflective,
    };

    enum class MaterialPrimitiveType
    {
        Triangle,
        Line
    };

    class Material;
}