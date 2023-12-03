
#pragma once

#include <bgfx/bgfx.h>
#include <bimg/bimg.h>

#include <string>
#include <memory>

namespace darmok
{
	class AssetContextImpl;

	class AssetContext final
	{
	public:
		static AssetContext& get();

		bgfx::ShaderHandle loadShader(const std::string& name);
		bgfx::ProgramHandle loadProgram(const std::string& vsName, const std::string& fsName);
		bgfx::TextureHandle loadTexture(const std::string& name, uint64_t flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE, uint8_t skip = 0, bgfx::TextureInfo* info = nullptr, bimg::Orientation::Enum* orientation = nullptr);
		bimg::ImageContainer* loadImage(const std::string& filePath, bgfx::TextureFormat::Enum dstFormat);

		AssetContextImpl& getImpl();
		const AssetContextImpl& getImpl() const;
	private:
		AssetContext();

		std::unique_ptr<AssetContextImpl> _impl;
	};
}