#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/texture_fwd.hpp>
#include <darmok/color_fwd.hpp>
#include <darmok/image_fwd.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/loader.hpp>
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

	using TextureConfig = protobuf::TextureConfig;

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
		[[nodiscard]] TextureConfig getTextureConfig() const noexcept;
		[[nodiscard]] DataView getData() const noexcept;		
		[[nodiscard]] bool isTextureValid(uint64_t flags = defaultTextureLoadFlags) const noexcept;
		[[nodiscard]] TextureType getTextureType() const noexcept;
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

	class DARMOK_EXPORT BX_NO_VTABLE IImageLoader : public ILoader<Image>
	{
	};

	class IDataLoader;

	class DARMOK_EXPORT ImageLoader final : public IImageLoader
	{
	public:
		ImageLoader(IDataLoader& dataLoader, bx::AllocatorI& alloc) noexcept;
		[[nodiscard]] std::shared_ptr<Image> operator()(std::filesystem::path path) override;
	private:
		IDataLoader& _dataLoader;
		bx::AllocatorI& _alloc;
	};

	class DARMOK_EXPORT ImageFileImporter final : public IFileTypeImporter
	{
	public:
		ImageFileImporter() noexcept;
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