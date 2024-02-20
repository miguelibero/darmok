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
		Image(bimg::ImageContainer* container);
		~Image();
		bool empty() const;

		glm::uvec2 getSize() const;
		uint32_t getDepth() const;
		bool isCubeMap() const;
		uint8_t getMipCount() const;
		uint16_t getLayerCount() const;
		bimg::TextureFormat::Enum getFormat() const;

		const bgfx::Memory* makeRef() const;
		bgfx::TextureInfo getTextureInfo() const;
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