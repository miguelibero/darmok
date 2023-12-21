
#pragma once

#include <bx/bx.h>
#include <bimg/bimg.h>
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

#include <vector>
#include <string>
#include <memory>

namespace darmok
{
	class AssetContextImpl;
	class Model;
	class Data;

	static const uint64_t defaultTextureCreationFlags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE;

	class Image final
	{
	public:
		Image(bimg::ImageContainer* container);
		~Image();

		bool empty() const;
		const bgfx::Memory* makeRef() const;
		bgfx::TextureHandle createTexture(uint64_t flags = defaultTextureCreationFlags) const;
		bgfx::TextureInfo calcTextureInfo() const;
	private:
		bimg::ImageContainer* _container;
	};

	class Texture final
	{
	public:
		Texture(const bgfx::TextureHandle& handle);
		const bgfx::TextureHandle& getHandle() const;
	private:
		bgfx::TextureHandle _handle;
	};

	typedef glm::vec<2, uint32_t> TextureVec2;

	struct TextureAtlasVertex
	{
		TextureVec2 position;
		TextureVec2 texCoord;
	};
		
	typedef uint16_t TextureAtlasVertexIndex;

	struct TextureBounds
	{
		TextureVec2 size;
		TextureVec2 offset;
	};

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

		TextureBounds getBounds() const;
	};

	struct TextureAtlas final
	{
		std::shared_ptr<Texture> texture;
		std::vector<TextureAtlasElement> elements;
		TextureVec2 size;

		TextureBounds getBounds(const std::string& prefix) const;
		TextureAtlasElement* getElement(const std::string& name);
		const TextureAtlasElement* getElement(const std::string& name) const;
	};

	struct Program final
	{
		Program(const bgfx::ProgramHandle& handle);
		const bgfx::ProgramHandle& getHandle() const;
	private:
		bgfx::ProgramHandle _handle;
	};

	class BX_NO_VTABLE IDataLoader
	{
	public:
		virtual ~IDataLoader() = default;
		virtual std::shared_ptr<Data> operator()(const std::string& name) = 0;
	};

	class BX_NO_VTABLE IImageLoader
	{
	public:
		virtual ~IImageLoader() = default;
		virtual std::shared_ptr<Image> operator()(const std::string& name, bimg::TextureFormat::Enum format = bimg::TextureFormat::Count) = 0;
	};

	class BX_NO_VTABLE IProgramLoader
	{
	public:
		virtual ~IProgramLoader() = default;
		virtual std::shared_ptr<Program> operator()(const std::string& vertexName, const std::string& fragmentName = "") = 0;
	};

	class BX_NO_VTABLE ITextureLoader
	{
	public:
		
		virtual ~ITextureLoader() = default;
		virtual std::shared_ptr<Texture> operator()(const std::string& name, uint64_t flags = defaultTextureCreationFlags) = 0;
	private:
	};

	class BX_NO_VTABLE ITextureAtlasLoader
	{
	public:
		virtual ~ITextureAtlasLoader() = default;
		virtual std::shared_ptr<TextureAtlas> operator()(const std::string& name, uint64_t flags = defaultTextureCreationFlags) = 0;
	};

	class BX_NO_VTABLE IModelLoader
	{
	public:
		virtual ~IModelLoader() = default;
		virtual std::shared_ptr<Model> operator()(const std::string& name) = 0;
	};

	class AssetContext final
	{
	public:
		static AssetContext& get() noexcept;

		IImageLoader& getImageLoader() noexcept;
		IProgramLoader& getProgramLoader() noexcept;
		ITextureLoader& getTextureLoader() noexcept;
		ITextureAtlasLoader& getTextureAtlasLoader() noexcept;
		IModelLoader& getModelLoader() noexcept;

		AssetContextImpl& getImpl() noexcept;
		const AssetContextImpl& getImpl() const noexcept;
	private:
		AssetContext() noexcept;

		std::unique_ptr<AssetContextImpl> _impl;
	};
}