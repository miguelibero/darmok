#pragma once

#include <darmok/texture.hpp>


namespace darmok
{
	class IDataLoader;

    class ImageTextureLoader final : public ITextureLoader
	{
	public:
		ImageTextureLoader(IImageLoader& imgLoader) noexcept;
		std::shared_ptr<Texture> operator()(std::string_view name, uint64_t flags = defaultTextureCreationFlags) noexcept override;
	private:
		IImageLoader& _imgLoader;
	};

    class TexturePackerTextureAtlasLoader final : public ITextureAtlasLoader
	{
	public:
		TexturePackerTextureAtlasLoader(IDataLoader& dataLoader, ITextureLoader& textureLoader) noexcept;
		std::shared_ptr<TextureAtlas> operator()(std::string_view name, uint64_t flags = defaultTextureCreationFlags) override;
	private:
		IDataLoader& _dataLoader;
		ITextureLoader& _textureLoader;
	};
}