
vec2 blinn(vec3 lightDir, vec3 normal, vec3 viewDir)
{
	float ndotl = dot(normal, lightDir);
	vec3 reflected = lightDir - 2.0 * ndotl* normal; // reflect(lightDir, normal);
	float rdotv = dot(reflected, viewDir);
	return vec2(ndotl, rdotv);
}

float fresnel(float ndotl, float bias, float powv)
{
	float facing = (1.0 - ndotl);
	return max(bias + (1.0 - bias) * pow(facing, powv), 0.0);
}

vec4 lit(float ndotl, float rdotv, float m)
{
	float diff = max(0.0, ndotl);
	float spec = step(0.0, ndotl) * max(0.0, rdotv * m);
	return vec4(1.0, diff, spec, 1.0);
}

vec4 powRgba(vec4 rgba, float powv)
{
	vec4 result;
	result.xyz = pow(rgba.xyz, vec3_splat(powv));
	result.w = rgba.w;
	return result;
}

vec3 calcLight(vec3 wpos, vec3 normal, vec3 view, vec3 lightPos, float lightRadius, vec3 lightRgb, float lightInner)
{
	vec3 lp = lightPos - wpos;
	float attn = 1.0 - smoothstep(lightInner, 1.0, length(lp) / lightRadius);
	vec3 lightDir = normalize(lp);
	vec2 bln = blinn(lightDir, normal, view);
	vec4 lc = lit(bln.x, bln.y, 1.0);
	vec3 rgb = lightRgb * saturate(lc.y) * attn;
	return rgb;
}

// bgfx examples utils

vec3 encodeNormalUint(vec3 normal)
{
	return normal * 0.5 + 0.5;
}

vec3 decodeNormalUint(vec3 encodedNormal)
{
	return encodedNormal * 2.0 - 1.0;
}

float toClipSpaceDepth(float depthTextureZ)
{
#if BGFX_SHADER_LANGUAGE_GLSL
	return depthTextureZ * 2.0 - 1.0;
#else
	return depthTextureZ;
#endif // BGFX_SHADER_LANGUAGE_GLSL
}

vec3 clipToWorld(mat4 invViewProj, vec3 clipPos)
{
	vec4 wpos = mul(invViewProj, vec4(clipPos, 1.0) );
	return wpos.xyz / wpos.w;
}

float toGamma(float r)
{
	return pow(abs(r), 1.0/2.2);
}

vec3 toGamma(vec3 rgb)
{
	return pow(abs(rgb), vec3_splat(1.0/2.2) );
}

vec4 toGamma(vec4 rgba)
{
	return vec4(toGamma(rgba.xyz), rgba.w);
}

vec3 toLinear(vec3 rgb)
{
	return pow(abs(rgb), vec3_splat(2.2) );
}

vec4 toLinear(vec4 rgba)
{
	return vec4(toLinear(rgba.xyz), rgba.w);
}
