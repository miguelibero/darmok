#ifndef DARMOK_SKINNING_HEADER
#define DARMOK_SKINNING_HEADER

mat4 getSkinningMatrix(vec4 indices, vec4 weights)
{
    return (u_model[int(indices.x) + 2] * weights.x) +
           (u_model[int(indices.y) + 2] * weights.y) +
           (u_model[int(indices.z) + 2] * weights.z) +
           (u_model[int(indices.w) + 2] * weights.w);
}

#endif