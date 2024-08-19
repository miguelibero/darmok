#ifndef DARMOK_MATERIAL_BASIC_HEADER
#define DARMOK_MATERIAL_BASIC_HEADER

#include <darmok_samplers.sc>

SAMPLER2D(s_texBaseColor,         DARMOK_SAMPLER_MATERIAL_BASECOLOR);
SAMPLER2D(s_texSpecular,          DARMOK_SAMPLER_MATERIAL_SPECULAR);

uniform vec4 u_baseColorFactor;
uniform vec4 u_specularFactorVec;
uniform vec4 u_hasTextures;

#define u_hasBaseColorTexture ((uint(u_hasTextures.x) & (1 << 0)) != 0)
#define u_hasSpecularTexture  ((uint(u_hasTextures.x) & (1 << 1)) != 0)

#define u_specularFactor (u_specularFactorVec.xyz)
#define u_shininess      (u_specularFactorVec.w)

struct Material
{
    vec4 diffuse;
    vec3 specular;
    float shininess;
};

vec4 materialDiffuseValue(vec2 texcoord)
{
    if(u_hasBaseColorTexture)
    {
        return texture2D(s_texBaseColor, texcoord) * u_baseColorFactor;
    }
    else
    {
        return u_baseColorFactor;
    }
}

vec3 materialSpecularValue(vec2 texcoord)
{
    if(u_hasSpecularTexture)
    {
        return texture2D(s_texSpecular, texcoord).rgb * u_specularFactor;
    }
    else
    {
        return u_specularFactor;
    }
}

Material getMaterial(vec2 texcoord)
{
    Material mat;

    mat.diffuse = materialDiffuseValue(texcoord);
    mat.specular = materialSpecularValue(texcoord);
    mat.shininess = u_shininess;

    return mat;
}

#endif // DARMOK_MATERIAL_BASIC_HEADER
