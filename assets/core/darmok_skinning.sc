#ifndef DARMOK_SKINNING_HEADER
#define DARMOK_SKINNING_HEADER

uniform mat4 u_skinning[64];

mat4 getSkinningMatrixPart(float v)
{
    return u_skinning[int(v) + 1];
}

mat4 getSkinningMatrix(vec4 indices, vec4 weights)
{
    return weights.x * getSkinningMatrixPart(indices.x) +
           weights.y * getSkinningMatrixPart(indices.y) +
           weights.z * getSkinningMatrixPart(indices.z) +
           weights.w * getSkinningMatrixPart(indices.w);
}

vec4 applySkinning(vec4 v, vec4 indices, vec4 weights)
{
#if DARMOK_VARIANT_SKINNING_ENABLED
    if(indices.x >= 0.0f)
    {
        mat4 skin = getSkinningMatrix(indices, weights);
        v = mul(skin, v);
    }
#endif
    return v;
}

#endif // DARMOK_SKINNING_HEADER