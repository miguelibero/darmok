#pragma once

#include <darmok/glm.hpp>
#include <bimg/bimg.h>
#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <bx/allocator.h>
#include <darmok/export.h>
#include <darmok/texture_fwd.hpp>
#include <darmok/color_fwd.hpp>
#include <darmok/image_fwd.hpp>
#include <darmok/asset_core.hpp>

#include <memory>
#include <string_view>
#include <unordered_map>
#include <iostream>
#include <filesystem>
#include <array>

namespace darmok
{
	class Data;
	class DataView;
	struct TextureConfig;

    class DARMOK_EXPORT Image final
	{
	public:
		Image(DataView data, bx::AllocatorI& alloc, bimg::TextureFormat::Enum format = bimg::TextureFormat::Count);
		Image(const Color& color, bx::AllocatorI& alloc, const glm::uvec2& size = { 1, 1 }) noexcept;
		Image(const glm::uvec2& size, bx::AllocatorI& alloc, bimg::TextureFormat::Enum format = bimg::TextureFormat::Count);
		Image(const std::array<DataView, 6>& faceData, bx::AllocatorI& alloc, bimg::TextureFormat::Enum format = bimg::TextureFormat::Count);
		Image(bimg::ImageContainer* container);
		~Image() noexcept;
		Image(Image&& other) noexcept;
		Image& operator=(Image&& other) noexcept;
		Image(const Image& other) noexcept;
		Image& operator=(const Image& other) noexcept;

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
		void encode(ImageEncoding encoding, bx::WriterI& writer) const noexcept;
		[[nodiscard]] Data encode(ImageEncoding encoding) const noexcept;
		void write(ImageEncoding encoding, std::ostream& stream) const noexcept;

		void update(const glm::uvec2& pos, const glm::uvec2& size, DataView data, size_t elmOffset = 0, size_t elmSize = 1);

		static bimg::TextureFormat::Enum readFormat(std::string_view name) noexcept;
		static ImageEncoding getEncodingForPath(const std::filesystem::path& path) noexcept;
		
	private:
		bimg::ImageContainer* _container;
		static const std::unordered_map<bimg::TextureFormat::Enum, std::string> _formatNames;

		void copyContainer(const Image& other) noexcept;
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

	class DARMOK_EXPORT ImageImporter final : public IAssetTypeImporter
	{
	public:
		bool startImport(const Input& input, bool dry = false) override;
		Outputs getOutputs(const Input& input) noexcept override;
		Dependencies getDependencies(const Input& input) noexcept override;
		void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) noexcept override;
		void endImport(const Input& input) noexcept override;
		const std::string& getName() const noexcept override;
	private:
		std::filesystem::path _outputPath;
		std::optional<std::array<std::filesystem::path, 6>> _cubemapFaces;
		ImageEncoding _outputEncoding;
		bx::DefaultAllocator _alloc;
	};
}