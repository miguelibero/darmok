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

	class Texture;
	struct TextureConfig;
	class ITextureLoader;
	class ColorTextureLoader;
}