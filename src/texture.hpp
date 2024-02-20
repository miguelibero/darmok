#pragma once

#include <darmok/texture.hpp>
#include <optional>
#include <pugixml.hpp>


namespace darmok
{
	class IDataLoader;

    class ImageTextureLoader final : public ITextureLoader
	{
	public:
		ImageTextureLoader(IImageLoader& imgLoader);
		std::shared_ptr<Texture> operator()(std::string_view name, uint64_t flags = defaultTextureCreationFlags) override;
	private:
		IImageLoader& _imgLoader;
		bx::AllocatorI* _allocator;
	};

    class TexturePackerTextureAtlasLoader final : public ITextureAtlasLoader
	{
	public:
		TexturePackerTextureAtlasLoader(IDataLoader& dataLoader, ITextureLoader& textureLoader);
		std::shared_ptr<TextureAtlas> operator()(std::string_view name, uint64_t flags = defaultTextureCreationFlags) override;
	private:
		IDataLoader& _dataLoader;
		ITextureLoader& _textureLoader;

		static std::pair<int, size_t> readXmlValueInt(std::string_view str, size_t i);
		static std::pair<std::optional<TextureVec2>, size_t> readXmlValueVec(std::string_view str, size_t i);
		static TextureAtlasElement loadElement(pugi::xml_node& xml);
	};
}