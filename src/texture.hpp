#pragma once

#include <darmok/texture.hpp>

namespace darmok
{
    class ImageTextureLoader final : public ITextureLoader
	{
	public:
		ImageTextureLoader(IImageLoader& imgLoader) noexcept;
		std::shared_ptr<Texture> operator()(std::string_view name, uint64_t flags = defaultTextureCreationFlags) noexcept override;
	private:
		IImageLoader& _imgLoader;
	};
}