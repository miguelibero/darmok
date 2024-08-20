#ifndef DARMOK_MATERIAL_BASIC_HEADER
#define DARMOK_MATERIAL_BASIC_HEADER

#include <darmok_sampler.sc>
#include <darmok_util.sc>
#include <bgfx_shader.sh>

SAMPLER2D(s_texBaseColor, DARMOK_SAMPLER_MATERIAL_BASECOLOR);
SAMPLER2D(s_texSpecular,  DARMOK_SAMPLER_MATERIAL_SPECULAR);

uniform vec4 u_baseColorFactor;
uniform vec4 u_specularFactorVec;
uniform vec4 u_hasTextures;

#define u_hasBaseColorTexture getBitfieldValue(u_hasTextures, 0)
#define u_hasSpecularTexture  getBitfieldValue(u_hasTextures, 1)

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

vec3 calcDiffuse(vec3 lightDir, vec3 normal, vec3 lightIntensity)
{
	float diff = max(dot(normal, -lightDir), 0.0);
	return diff * lightIntensity;
}

vec3 calcSpecular(vec3 lightDir, vec3 normal, vec3 viewDir, vec3 lightIntensity, float shininess)
{
    vec3 reflectDir = reflect(lightDir, normal);  
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	return spec * lightIntensity;
}

#endif // DARMOK_MATERIAL_BASIC_HEADER
