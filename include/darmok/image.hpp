#pragma once

#include <glm/glm.hpp>
#include <bimg/bimg.h>
#include <bgfx/bgfx.h>
#include <bx/bx.h>

#include <memory>
#include <string_view>

namespace darmok
{
    class Image final
	{
	public:
		Image(bimg::ImageContainer* container) noexcept;
		~Image() noexcept;
		Image(const Image& other) = delete;
		Image& operator=(const Image& other) = delete;
		
		[[nodiscard]] bool empty() const noexcept;
		[[nodiscard]] glm::uvec2 getSize() const noexcept;
		[[nodiscard]] uint32_t getDepth() const noexcept;
		[[nodiscard]] bool isCubeMap() const noexcept;
		[[nodiscard]] uint8_t getMipCount() const noexcept;
		[[nodiscard]] uint16_t getLayerCount() const noexcept;
		[[nodiscard]] bimg::TextureFormat::Enum getFormat() const noexcept;
		[[nodiscard]] bgfx::TextureInfo getTextureInfo() const noexcept;
		[[nodiscard]] const bgfx::Memory* makeRef() const noexcept;

	private:
		bimg::ImageContainer* _container;
	};

	class BX_NO_VTABLE IImageLoader
	{
	public:
		virtual ~IImageLoader() = default;
		virtual std::shared_ptr<Image> operator()(std::string_view name) = 0;
	};
}