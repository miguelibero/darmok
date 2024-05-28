#ifndef DARMOK_SKINNING_HEADER
#define DARMOK_SKINNING_HEADER

mat4 getSkinningMatrix(vec4 indices, vec4 weights)
{
    return weights.x * u_model[int(indices.x)] +
           weights.y * u_model[int(indices.y)] +
           weights.z * u_model[int(indices.z)] +
           weights.w * u_model[int(indices.w)];
}

#endif