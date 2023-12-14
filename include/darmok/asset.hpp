
#pragma once

#include <bgfx/bgfx.h>
#include <bimg/bimg.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <memory>

namespace darmok
{
	class AssetContextImpl;

	class Image
	{
	public:
		Image(bimg::ImageContainer* container);
		~Image();
	private:
		bimg::ImageContainer* _container;
	};

	struct TextureWithInfo
	{
		bgfx::TextureHandle texture;
		bgfx::TextureInfo info;
		bimg::Orientation::Enum orientation;
	};

	typedef glm::vec<2, uint32_t> TextureVec2;

	struct TextureAtlasVertex
	{
		TextureVec2 position;
		TextureVec2 texCoord;
	};
		
	typedef uint16_t TextureAtlasVertexIndex;

	struct TextureAtlasElement final
	{
		std::string name;
		std::vector<TextureAtlasVertex> vertices;
		std::vector<TextureAtlasVertexIndex> indices;
		TextureVec2 texturePosition;
		TextureVec2 textureSize;
		TextureVec2 originalPosition;
		TextureVec2 originalSize;
		TextureVec2 pivot;
		bool rotated;
	};

	struct TextureAtlas final
	{
		TextureVec2 size;
		std::string name;
		bgfx::TextureHandle texture;
		std::vector<TextureAtlasElement> elements;

		TextureAtlasElement* getElement(const std::string& name);
		const TextureAtlasElement* getElement(const std::string& name) const;
	};

	class AssetContext final
	{
	public:
		static AssetContext& get() noexcept;

		bgfx::ShaderHandle loadShader(const std::string& name);
		bgfx::ProgramHandle loadProgram(const std::string& vertexName, const std::string& fragmentName = "");
		bgfx::TextureHandle loadTexture(const std::string& name, uint64_t flags = _defaultTextureFlags);
		TextureWithInfo loadTextureWithInfo(const std::string& name, uint64_t flags = _defaultTextureFlags);
		Image loadImage(const std::string& filePath, bgfx::TextureFormat::Enum dstFormat = bgfx::TextureFormat::Count);
		TextureAtlas loadAtlas(const std::string& filePath, uint64_t flags = _defaultTextureFlags);

		AssetContextImpl& getImpl() noexcept;
		const AssetContextImpl& getImpl() const noexcept;
	private:
		static const uint64_t _defaultTextureFlags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE;

		AssetContext() noexcept;

		std::unique_ptr<AssetContextImpl> _impl;
	};
}