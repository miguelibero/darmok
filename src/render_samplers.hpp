#pragma once

namespace darmok
{
    struct RenderSamplers final
    {
        // these should be the same as in darmok_samplers.sc
        static const uint8_t MATERIAL_ALBEDO_LUT = 0;

        static const uint8_t MATERIAL_ALBEDO = 1;
        static const uint8_t MATERIAL_SPECULAR = 2;
        static const uint8_t MATERIAL_METALLIC_ROUGHNESS = 3;
        static const uint8_t MATERIAL_NORMAL = 4;
        static const uint8_t MATERIAL_OCCLUSION = 5;
        static const uint8_t MATERIAL_EMISSIVE = 6;

        static const uint8_t LIGHTS_POINT = 7;
        static const uint8_t LIGHTS_DIR = 8;
        static const uint8_t LIGHTS_SPOT = 9;

        static const uint8_t SHADOW_MAP = 10;
        static const uint8_t SHADOW_TRANS = 11;

        static const uint8_t CLUSTERS_CLUSTERS = 12;
        static const uint8_t CLUSTERS_LIGHTINDICES = 13;
        static const uint8_t CLUSTERS_LIGHTGRID = 14;
        static const uint8_t CLUSTERS_ATOMICINDEX = 15;

        static const uint8_t DEFERRED_DIFFUSE_A = 12;
        static const uint8_t DEFERRED_NORMAL = 13;
        static const uint8_t DEFERRED_F0_METALLIC = 14;
        static const uint8_t DEFERRED_EMISSIVE_OCCLUSION = 15;
        static const uint8_t DEFERRED_DEPTH = 16;


    };
}