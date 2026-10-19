#pragma once

// Used by forward-shaded passes that draw on top of a deferred scene
// (eg. transparent objects). The opaque geometry depth is read from the
// deferred G-buffer and fragments behind it are discarded.
#ifdef DARMOK_VARIANT_SCENE_DEPTH_ENABLED

[[vk::binding(15, 0)]]
uniform Texture2D<float> s_sceneDepth : register(t15);

#endif

void discardIfHiddenBySceneDepth(float4 fragCoord)
{
#ifdef DARMOK_VARIANT_SCENE_DEPTH_ENABLED
    float sceneDepth = s_sceneDepth.Load(int3(int2(fragCoord.xy), 0));
    if (fragCoord.z > sceneDepth)
    {
        discard;
    }
#endif
}
