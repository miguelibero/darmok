#ifndef DARMOK_SKINNING_HEADER
#define DARMOK_SKINNING_HEADER

mat4 getSkinningMatrix(vec4 indices, vec4 weights)
{
    return (u_model[int(indices.x) + 1] * weights.x) +
           (u_model[int(indices.y) + 1] * weights.y) +
           (u_model[int(indices.z) + 1] * weights.z) +
           (u_model[int(indices.w) + 1] * weights.w);
}

#endif