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

mat4 getSkinnedModelMatrix(vec4 indices, vec4 weights)
{
    mat4 model = u_model[0];
    // TODO: optimize avoiding if
    // adding an identity matrix in the beginning
    // or different shaders
    if(indices.x >= 0.0f)
    {
        model *= getSkinningMatrix(indices, weights);
    }
    return model;
}

vec4 applySkinning(vec4 v, vec4 indices, vec4 weights)
{
    if(indices.x >= 0.0f)
    {
        v = mul(getSkinningMatrix(indices, weights), v);
    }
    return v;
}

#endif // DARMOK_SKINNING_HEADER