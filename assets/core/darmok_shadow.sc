
SAMPLER2D(s_shadowMap, DARMOK_SAMPLER_SHADOW_MAP);

float calcShadow(mat4 lightSpaceMatrix)
{
	vec4 fragPosLightSpace = lightSpaceMatrix * vec4(v_position, 1.0)
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(s_shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}