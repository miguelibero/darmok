#pragma once

#include <glm/glm.hpp>
#include <bimg/bimg.h>
#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <darmok/texture_fwd.hpp>
#include <darmok/color_fwd.hpp>

#include <memory>
#include <string_view>
#include <unordered_map>

namespace darmok
{
	class DataView;
	struct TextureConfig;

    class Image final
	{
	public:
		DLLEXPORT Image(const DataView& data, bx::AllocatorI& alloc, bimg::TextureFormat::Enum format = bimg::TextureFormat::Count);
		DLLEXPORT Image(const Color& color, bx::AllocatorI& alloc, const glm::uvec2& size = { 1, 1 }) noexcept;
		DLLEXPORT Image(bimg::ImageContainer* container);
		DLLEXPORT ~Image() noexcept;
		DLLEXPORT Image(const Image& other) = delete;
		DLLEXPORT Image& operator=(const Image& other) = delete;

		[[nodiscard]] DLLEXPORT bool empty() const noexcept;
		[[nodiscard]] DLLEXPORT glm::uvec2 getSize() const noexcept;
		[[nodiscard]] DLLEXPORT uint32_t getDepth() const noexcept;
		[[nodiscard]] DLLEXPORT bool isCubeMap() const noexcept;
		[[nodiscard]] DLLEXPORT uint8_t getMipCount() const noexcept;
		[[nodiscard]] DLLEXPORT uint16_t getLayerCount() const noexcept;
		[[nodiscard]] DLLEXPORT bimg::TextureFormat::Enum getFormat() const noexcept;
		[[nodiscard]] DLLEXPORT bgfx::TextureInfo getTextureInfo() const noexcept;
		[[nodiscard]] DLLEXPORT TextureConfig getTextureConfig(uint64_t flags = 0) const noexcept;
		[[nodiscard]] DLLEXPORT DataView getData() const noexcept;
		[[nodiscard]] DLLEXPORT TextureType getTextureType(uint64_t flags) const noexcept;
		[[nodiscard]] DLLEXPORT bx::AllocatorI& getAllocator() const noexcept;

	private:
		bimg::ImageContainer* _container;
	};

	class BX_NO_VTABLE IImageLoader
	{
	public:
		using result_type = std::shared_ptr<Image>;
		DLLEXPORT virtual ~IImageLoader() = default;
		DLLEXPORT virtual result_type operator()(std::string_view name) = 0;
	};

	class IDataLoader;

	class DataImageLoader final : public IImageLoader
	{
	public:
		DataImageLoader(IDataLoader& dataLoader, bx::AllocatorI& alloc) noexcept;
		[[nodiscard]] std::shared_ptr<Image> operator()(std::string_view name) override;
	private:
		IDataLoader& _dataLoader;
		bx::AllocatorI& _allocator;
	};
}