#pragma once

#ifdef DARMOK_VARIANT_SKINNING_ENABLED
uniform float4x4 u_skinning[64];

float4x4 getSkinningMatrixPart(float v)
{
    return u_skinning[int(v + 1)];
}

float4x4 getSkinningMatrix(float4 indices, float4 weights)
{
    return weights.x * getSkinningMatrixPart(indices.x) +
           weights.y * getSkinningMatrixPart(indices.y) +
           weights.z * getSkinningMatrixPart(indices.z) +
           weights.w * getSkinningMatrixPart(indices.w);
}

float4 applySkinning(float4 v, float4 indices, float4 weights)
{
    if (indices.x >= 0)
    {
        float4x4 skin = getSkinningMatrix(indices, weights);
        return mul(skin, v);
    }
    return v;
}

#else

float4 applySkinning(float4 v, float4 indices, float4 weights)
{
    return v;
}

#endif

