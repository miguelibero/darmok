#pragma once

namespace darmok
{
    struct RenderSamplers final
    {
        // these should be the same as in the shaders

        static const uint8_t MATERIAL_ALBEDO = 0;
        static const uint8_t MATERIAL_SPECULAR = 1;
        static const uint8_t MATERIAL_METALLIC_ROUGHNESS = 2;
        static const uint8_t MATERIAL_NORMAL = 3;
        static const uint8_t MATERIAL_OCCLUSION = 4;
        static const uint8_t MATERIAL_EMISSIVE = 5;
        static const uint8_t MATERIAL_ENV_IRRADIANCE = 6;
        static const uint8_t MATERIAL_ENV_PREFILTERED = 7;

        static const uint8_t PBR_ALBEDO_LUT = 8;
        static const uint8_t PBR_BRDF_LUT = 9;

        static const uint8_t LIGHTS_DIR = 10;
        static const uint8_t LIGHTS_POINT = 11;
        static const uint8_t LIGHTS_SPOT = 12;

        static const uint8_t SHADOW_MAP = 13;
        static const uint8_t SHADOW_TRANS = 14;
    };
}
