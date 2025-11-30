#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/color_fwd.hpp>
#include <darmok/image_fwd.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/loader.hpp>
#include <darmok/expected.hpp>
#include <darmok/protobuf/texture.pb.h>

#include <memory>
#include <string_view>
#include <unordered_map>
#include <iostream>
#include <filesystem>
#include <array>

#include <bimg/bimg.h>
#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <bx/allocator.h>


namespace darmok
{
	class Data;
	class DataView;

    class DARMOK_EXPORT Image final
	{
	public:
		using TextureConfig = protobuf::TextureConfig;
		using TextureType = protobuf::Texture::Type;

		Image(const Color& color, bx::AllocatorI& alloc, const glm::uvec2& size = { 1, 1 }) noexcept;
		Image(const glm::uvec2& size, bx::AllocatorI& alloc, bimg::TextureFormat::Enum format = bimg::TextureFormat::Count) noexcept;
		Image(bimg::ImageContainer* container) noexcept;
		~Image() noexcept;
		Image(const Image& other) noexcept;
		Image& operator=(const Image& other) noexcept;
		Image(Image&& other) noexcept;
		Image& operator=(Image&& other) noexcept;

		static expected<Image, std::string> load(DataView data, bx::AllocatorI& alloc, bimg::TextureFormat::Enum format = bimg::TextureFormat::Count) noexcept;
		static expected<Image, std::string> load(const std::array<DataView, 6>& faceData, bx::AllocatorI& alloc, bimg::TextureFormat::Enum format = bimg::TextureFormat::Count) noexcept;

		[[nodiscard]] bool empty() const noexcept;
		[[nodiscard]] glm::uvec2 getSize() const noexcept;
		[[nodiscard]] uint32_t getDepth() const noexcept;
		[[nodiscard]] bool isCubeMap() const noexcept;
		[[nodiscard]] uint8_t getMipCount() const noexcept;
		[[nodiscard]] uint16_t getLayerCount() const noexcept;
		[[nodiscard]] bimg::TextureFormat::Enum getFormat() const noexcept;
		[[nodiscard]] bgfx::TextureInfo getTextureInfo() const noexcept;
		[[nodiscard]] TextureConfig getTextureConfig() const noexcept;
		[[nodiscard]] DataView getData() const noexcept;
		[[nodiscard]] expected<void, std::string> setData(DataView data) noexcept;
		[[nodiscard]] bool isTextureValid(uint64_t flags) const noexcept;
		[[nodiscard]] TextureType getTextureType() const noexcept;
		[[nodiscard]] bx::AllocatorI& getAllocator() const noexcept;
		[[nodiscard]] expected<void, std::string> encode(ImageEncoding encoding, bx::WriterI& writer) const noexcept;
		[[nodiscard]] expected<Data, std::string> encode(ImageEncoding encoding) const noexcept;
		[[nodiscard]] expected<void, std::string> write(ImageEncoding encoding, std::ostream& stream) const noexcept;

		[[nodiscard]] expected<void, std::string> update(const glm::uvec2& pos, const glm::uvec2& size, DataView data, size_t elmOffset = 0, size_t elmSize = 1);

		static bimg::TextureFormat::Enum readFormat(std::string_view name) noexcept;
		static ImageEncoding readEncoding(std::string_view name) noexcept;
		static ImageEncoding getEncodingForPath(const std::filesystem::path& path) noexcept;
		
	private:
		bimg::ImageContainer* _container;

		void copyContainer(const Image& other) noexcept;
	};

	class DARMOK_EXPORT BX_NO_VTABLE IImageLoader : public ILoader<Image>{};

	class IDataLoader;

	class DARMOK_EXPORT ImageLoader final : public IImageLoader
	{
	public:
		ImageLoader(IDataLoader& dataLoader, bx::AllocatorI& alloc) noexcept;
		[[nodiscard]] Result operator()(std::filesystem::path path) noexcept override;
	private:
		IDataLoader& _dataLoader;
		bx::AllocatorI& _alloc;
	};

	class DARMOK_EXPORT ImageFileImporter final : public IFileTypeImporter
	{
	public:
		ImageFileImporter() noexcept;
		const std::string& getName() const noexcept override;

		expected<Effect, std::string> prepare(const Input& input) noexcept override;
		expected<void, std::string> operator()(const Input& input, Config& config) noexcept override;

	private:
		std::optional<std::array<std::filesystem::path, 6>> _cubemapFaces;
		ImageEncoding _outputEncoding;
		bx::DefaultAllocator _alloc;
	};
}