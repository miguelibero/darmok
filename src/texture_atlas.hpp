#pragma once

#include <darmok/texture_atlas.hpp>

namespace darmok
{
	class IDataLoader;

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