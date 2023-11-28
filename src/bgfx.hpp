
#pragma once

#include <bx/pixelformat.h>
#include <bgfx/bgfx.h>
#include <bimg/bimg.h>

#include <string>

namespace darmok
{
	///
	bgfx::ShaderHandle loadShader(const std::string& name);

	///
	bgfx::ProgramHandle loadProgram(const std::string& vsName, const std::string& fsName);

	///
	bgfx::TextureHandle loadTexture(const std::string& name, uint64_t flags = BGFX_TEXTURE_NONE|BGFX_SAMPLER_NONE, uint8_t skip = 0, bgfx::TextureInfo* info = NULL, bimg::Orientation::Enum* orientation = NULL);

	///
	bimg::ImageContainer* imageLoad(const std::string& filePath, bgfx::TextureFormat::Enum dstFormat);

	/// Returns true if both internal transient index and vertex buffer have
	/// enough space.
	///
	/// @param[in] _numVertices Number of vertices.
	/// @param[in] _layout Vertex layout.
	/// @param[in] _numIndices Number of indices.
	///
	inline bool checkAvailTransientBuffers(uint32_t numVertices, const bgfx::VertexLayout& layout, uint32_t numIndices)
	{
		return numVertices == bgfx::getAvailTransientVertexBuffer(numVertices, layout)
			&& (0 == numIndices || numIndices == bgfx::getAvailTransientIndexBuffer(numIndices) )
			;
	}

	///
	inline uint32_t encodeNormalRgba8(float x, float y = 0.0f, float z = 0.0f, float w = 0.0f)
	{
		const float src[] =
		{
			x * 0.5f + 0.5f,
			y * 0.5f + 0.5f,
			z * 0.5f + 0.5f,
			w * 0.5f + 0.5f,
		};
		uint32_t dst;
		bx::packRgba8(&dst, src);
		return dst;
	}
}