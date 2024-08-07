#pragma once

#include <bgfx/bgfx.h>

namespace darmok
{
	static const uint64_t defaultTextureLoadFlags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE;

	enum class TextureType
	{
		Unknown,
		CubeMap,
		Texture2D,
		Texture3D,
	};

    enum class TextureSamplingType
    {
        Min, // minification
        Mag, // magnification
        Mip  // mip level
    };

    enum class TextureSamplingMode
    {
        Anisotropic,
        Point
    };
}