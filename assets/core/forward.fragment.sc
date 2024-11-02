$input v_position, v_normal, v_tangent, v_texcoord0, v_viewDir

// all unit-vectors need to be normalized in the fragment shader, the interpolation of vertex shader output doesn't preserve length

// define samplers and uniforms for retrieving material parameters
#define READ_MATERIAL

#include <bgfx_shader.sh>
#include <bgfx_compute.sh>
#include <darmok_util.sc>
#include <darmok_material.sc>
#include <darmok_light.sc>

#if DARMOK_VARIANT_SHADOW_ENABLED
#include <darmok_shadow.sc>
#endif

void main()
{
    Material mat = getMaterial(v_texcoord0);
    // convert normal map from tangent space -> world space (= space of v_tangent, etc.)
    vec3 N = convertTangentNormal(v_normal, v_tangent, mat.normal);
    mat.a = specularAntiAliasing(N, mat.a);

    vec3 fragPos = v_position;
    vec3 V = v_viewDir;
    float NoV = abs(dot(N, V)) + 1e-5;

    if(whiteFurnaceEnabled())
    {
        mat.F0 = vec3_splat(1.0);
        vec3 msFactor = multipleScatteringFactor(mat, NoV);
        vec3 radianceOut = whiteFurnace(NoV, mat) * msFactor;
        gl_FragColor = vec4(radianceOut, 1.0);
        return;
    }

    vec3 msFactor = multipleScatteringFactor(mat, NoV);
    vec3 radianceOut = vec3_splat(0.0);

#if DARMOK_VARIANT_SHADOW_ENABLED
	float visibility = 0;
#endif

    uint pointLights = pointLightCount();
    for(uint i = 0; i < pointLights; i++)
    {
        PointLight light = getPointLight(i);
        float dist = distance(light.position, fragPos);
        float attenuation = smoothAttenuation(dist, light.range);
        if(attenuation > 0.0)
        {
            vec3 L = normalize(light.position - fragPos);
            vec3 radianceIn = light.intensity * attenuation;
            float NoL = saturate(dot(N, L));
            radianceOut += BRDF(V, L, N, NoV, NoL, mat) * msFactor * radianceIn * NoL;
        }
    }

    uint spotLights = spotLightCount();
    for(uint i = 0; i < spotLights; i++)
    {
        SpotLight light = getSpotLight(i);
        vec3 L = normalize(light.position - fragPos);
        float cosTheta = dot(L, normalize(-light.direction));
        float innerCutoff = cos(light.innerConeAngle * 0.5);
        float outerCutoff = cos(light.coneAngle * 0.5);

        float attenuation = 0.0;
        if (cosTheta > outerCutoff)
        {
            // Calculate the spotlight intensity with smooth edge falloff
            float factor = (cosTheta - outerCutoff) / (innerCutoff - outerCutoff);
            float dist = length(L);
            attenuation = factor * smoothAttenuation(dist, light.range);
        }

        if(attenuation > 0.0)
        {
            vec3 radianceIn = light.intensity * attenuation;
            float NoL = saturate(dot(N, L));
            radianceOut += BRDF(V, L, N, NoV, NoL, mat) * msFactor * radianceIn * NoL;
        }
    }

    uint dirLights = dirLightCount();
    for(uint i = 0; i < dirLights; i++)
    {
        DirectionalLight light = getDirLight(i);
        
        vec3 L = -light.direction;
        vec3 radianceIn = light.intensity;
        float NoL = saturate(dot(N, L));
        radianceOut += BRDF(V, L, N, NoV, NoL, mat) * msFactor * radianceIn * NoL;

#if DARMOK_VARIANT_SHADOW_ENABLED
		float shadowBias = normalShadowBias(N, L);  
		visibility += softShadow(i, fragPos, shadowBias);
        // visibility += hardShadow(i, fragPos, shadowBias);
#endif
    }

#if DARMOK_VARIANT_SHADOW_ENABLED
    radianceOut *= visibility / dirLights;
#endif

    radianceOut += getAmbientLight().irradiance * mat.diffuse * mat.occlusion;
    radianceOut += mat.emissive;

    gl_FragColor.rgb = radianceOut;
    gl_FragColor.a = mat.albedo.a;
}
