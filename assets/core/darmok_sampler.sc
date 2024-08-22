#ifndef DARMOK_SAMPLERS_HEADER
#define DARMOK_SAMPLERS_HEADER

// shared

#define DARMOK_SAMPLER_MATERIAL_ALBEDO_LUT 0

#define DARMOK_SAMPLER_MATERIAL_BASECOLOR 1
#define DARMOK_SAMPLER_MATERIAL_SPECULAR 2
#define DARMOK_SAMPLER_MATERIAL_METALLIC_ROUGHNESS 3
#define DARMOK_SAMPLER_MATERIAL_NORMAL 4
#define DARMOK_SAMPLER_MATERIAL_OCCLUSION 5
#define DARMOK_SAMPLER_MATERIAL_EMISSIVE 6

#define DARMOK_SAMPLER_LIGHTS_POINT 7
#define DARMOK_SAMPLER_LIGHTS_DIR 8
#define DARMOK_SAMPLER_LIGHTS_SPOT 9

#define DARMOK_SAMPLER_SHADOW_MAP 10
#define DARMOK_SAMPLER_SHADOW_TRANS 11

// per renderer

#define DARMOK_SAMPLER_CLUSTERS_CLUSTERS 12
#define DARMOK_SAMPLER_CLUSTERS_LIGHTINDICES 13
#define DARMOK_SAMPLER_CLUSTERS_LIGHTGRID 14
#define DARMOK_SAMPLER_CLUSTERS_ATOMICINDEX 15

#define DARMOK_SAMPLER_DEFERRED_DIFFUSE_A 12
#define DARMOK_SAMPLER_DEFERRED_NORMAL 13
#define DARMOK_SAMPLER_DEFERRED_F0_METALLIC 14
#define DARMOK_SAMPLER_DEFERRED_EMISSIVE_OCCLUSION 15
#define DARMOK_SAMPLER_DEFERRED_DEPTH 16

#endif // DARMOK_SAMPLERS_HEADER
