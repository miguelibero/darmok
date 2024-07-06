#pragma once

#include <darmok/glm.hpp>
#include <bimg/bimg.h>
#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <darmok/export.h>
#include <darmok/texture_fwd.hpp>
#include <darmok/color_fwd.hpp>

#include <memory>
#include <string_view>
#include <unordered_map>

namespace darmok
{
	class DataView;
	struct TextureConfig;

    class DARMOK_EXPORT Image final
	{
	public:
		Image(DataView data, bx::AllocatorI& alloc, bimg::TextureFormat::Enum format = bimg::TextureFormat::Count);
		Image(const Color& color, bx::AllocatorI& alloc, const glm::uvec2& size = { 1, 1 }) noexcept;
		Image(const glm::uvec2& size, bx::AllocatorI& alloc, bimg::TextureFormat::Enum format = bimg::TextureFormat::Count);
		Image(bimg::ImageContainer* container);
		~Image() noexcept;
		Image(Image&& other) noexcept;
		Image& operator=(Image&& other) noexcept;
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
		[[nodiscard]] TextureConfig getTextureConfig(uint64_t flags = defaultTextureLoadFlags) const noexcept;
		[[nodiscard]] DataView getData() const noexcept;		
		[[nodiscard]] TextureType getTextureType(uint64_t flags = defaultTextureLoadFlags) const noexcept;
		[[nodiscard]] bx::AllocatorI& getAllocator() const noexcept;

		void update(const glm::uvec2& pos, const glm::uvec2& size, DataView data);

	private:
		bimg::ImageContainer* _container;
	};

	class DARMOK_EXPORT BX_NO_VTABLE IImageLoader
	{
	public:
		using result_type = std::shared_ptr<Image>;
		virtual ~IImageLoader() = default;
		virtual result_type operator()(std::string_view name) = 0;
	};

	class IDataLoader;

	class DARMOK_EXPORT DataImageLoader final : public IImageLoader
	{
	public:
		DataImageLoader(IDataLoader& dataLoader, bx::AllocatorI& alloc) noexcept;
		[[nodiscard]] std::shared_ptr<Image> operator()(std::string_view name) override;
	private:
		IDataLoader& _dataLoader;
		bx::AllocatorI& _allocator;
	};
}