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

        static const uint8_t LIGHTS_POINTLIGHTS = 7;

        static const uint8_t CLUSTERS_CLUSTERS = 8;
        static const uint8_t CLUSTERS_LIGHTINDICES = 9;
        static const uint8_t CLUSTERS_LIGHTGRID = 10;
        static const uint8_t CLUSTERS_ATOMICINDEX = 11;

        static const uint8_t DEFERRED_DIFFUSE_A = 8;
        static const uint8_t DEFERRED_NORMAL = 9;
        static const uint8_t DEFERRED_F0_METALLIC = 10;
        static const uint8_t DEFERRED_EMISSIVE_OCCLUSION = 11;
        static const uint8_t DEFERRED_DEPTH = 12;


    };
}