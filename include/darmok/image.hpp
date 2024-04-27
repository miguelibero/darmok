#pragma once

#include <glm/glm.hpp>
#include <bimg/bimg.h>
#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <darmok/texture_fwd.hpp>

#include <memory>
#include <string_view>
#include <unordered_map>

namespace darmok
{
	using Color = glm::u8vec4;

    class Image final
	{
	public:
		Image(bimg::ImageContainer* container) noexcept;
		~Image() noexcept;
		Image(const Image& other) = delete;
		Image& operator=(const Image& other) = delete;

		static std::shared_ptr<Image> create(bx::AllocatorI* alloc, const Color& color, const glm::uvec2& size = { 1, 1 }) noexcept;

		[[nodiscard]] bool empty() const noexcept;
		[[nodiscard]] glm::uvec2 getSize() const noexcept;
		[[nodiscard]] uint32_t getDepth() const noexcept;
		[[nodiscard]] bool isCubeMap() const noexcept;
		[[nodiscard]] uint8_t getMipCount() const noexcept;
		[[nodiscard]] uint16_t getLayerCount() const noexcept;
		[[nodiscard]] bimg::TextureFormat::Enum getFormat() const noexcept;
		[[nodiscard]] bgfx::TextureInfo getTextureInfo() const noexcept;
		[[nodiscard]] const bgfx::Memory* makeRef() const noexcept;
		[[nodiscard]] const bgfx::Memory* copyMem() const noexcept;
		[[nodiscard]] TextureType getTextureType(uint64_t flags) const noexcept;

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