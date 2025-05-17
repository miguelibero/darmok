#pragma once

#include <darmok/export.h>
#include <darmok/image.hpp>
#include <darmok/color.hpp>
#include <darmok/glm.hpp>
#include <darmok/data.hpp>
#include <darmok/loader.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/utils.hpp>
#include <darmok/expected.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/protobuf/texture.pb.h>

#include <bgfx/bgfx.h>
#include <bx/bx.h>

#include <string>
#include <memory>

namespace darmok
{
	const uint64_t defaultTextureLoadFlags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE;

	namespace TextureUtils
	{
		void loadImage(protobuf::Texture& def, const Image& img) noexcept;
		Image createImage(const protobuf::Texture& def, bx::AllocatorI& alloc) noexcept;
		expected<void, std::string> writeImage(const protobuf::Texture& def, bx::AllocatorI& alloc, const std::filesystem::path& path) noexcept;
		bgfx::TextureInfo getInfo(const protobuf::TextureConfig& config) noexcept;
		const protobuf::TextureConfig& getEmptyConfig() noexcept;
	}

	class DARMOK_EXPORT BX_NO_VTABLE ITextureDefinitionLoader : public ILoader<protobuf::Texture>
	{
	};

	using DataTextureDefinitionLoader = DataProtobufLoader<ITextureDefinitionLoader>;

	class DARMOK_EXPORT ImageTextureDefinitionLoader final : public ITextureDefinitionLoader
	{
	public:
		ImageTextureDefinitionLoader(IImageLoader& imgLoader) noexcept;
		bool supports(const std::filesystem::path& path) const noexcept;
		ImageTextureDefinitionLoader& setLoadFlags(uint64_t flags = defaultTextureLoadFlags) noexcept;
		[[nodiscard]] Result operator()(std::filesystem::path path) override;
	private:
		IImageLoader& _imgLoader;
		uint64_t _loadFlags;
	};

	class DARMOK_EXPORT Texture final
	{
	public:
		using Config = protobuf::TextureConfig;
		using Definition = protobuf::Texture;
		using TextureType = protobuf::TextureType;
		using Format = protobuf::TextureFormat;

		Texture(const bgfx::TextureHandle& handle, const Config& cfg) noexcept;
		Texture(const Config& cfg, uint64_t flags = defaultTextureLoadFlags);
		Texture(const Image& img, uint64_t flags = defaultTextureLoadFlags);
		Texture(const DataView& data, const Config& cfg, uint64_t flags = defaultTextureLoadFlags);
		Texture(const Definition& definition) noexcept;
		Texture(Texture&& other) noexcept;
		Texture& operator=(Texture&& other) noexcept;

		~Texture() noexcept;
		Texture(const Texture& other) = delete;
		Texture& operator=(const Texture& other) = delete;

		[[nodiscard]] expected<void, std::string> update(const DataView& data, uint8_t mip = 0);
		[[nodiscard]] expected<void, std::string> update(const DataView& data, const glm::uvec2& size, const glm::uvec2& origin = glm::uvec2(0), uint8_t mip = 0, uint16_t layer = 0, uint8_t side = 0);
		[[nodiscard]] expected<void, std::string> update(const DataView& data, const glm::uvec3& size, const glm::uvec3& origin = glm::uvec3(0), uint8_t mip = 0);
		[[nodiscard]] uint32_t read(Data& data) noexcept;

		[[nodiscard]] std::string toString() const noexcept;
		[[nodiscard]] const bgfx::TextureHandle& getHandle() const noexcept;
		[[nodiscard]] TextureType::Enum getType() const noexcept;
		[[nodiscard]] glm::uvec2 getSize() const noexcept;
		[[nodiscard]] bgfx::TextureFormat::Enum getFormat() const noexcept;
		[[nodiscard]] uint16_t getLayerCount() const noexcept;
		[[nodiscard]] uint16_t getDepth() const noexcept;
		[[nodiscard]] bool hasMips() const noexcept;
		[[nodiscard]] uint32_t getStorageSize() const noexcept;
		[[nodiscard]] uint8_t getMipsCount() const noexcept;
		[[nodiscard]] uint8_t getBitsPerPixel() const noexcept;

		Texture& setName(std::string_view name) noexcept;

		[[nodiscard]] static std::string_view getFormatName(bgfx::TextureFormat::Enum format) noexcept;
		[[nodiscard]] static std::optional<bgfx::TextureFormat::Enum> readFormat(std::string_view name) noexcept;
		[[nodiscard]] static std::string_view getTypeName(TextureType::Enum type) noexcept;
		[[nodiscard]] static std::optional<TextureType::Enum> readType(std::string_view name) noexcept;
		[[nodiscard]] static uint64_t readFlags(const nlohmann::json& json) noexcept;

		using FlagMap = std::unordered_map<std::string, uint64_t>;

		[[nodiscard]] static const std::string& getTextureFlagName(uint64_t flag) noexcept;
		[[nodiscard]] static std::optional<uint64_t> readTextureFlag(std::string_view name) noexcept;
		[[nodiscard]] static const FlagMap& getTextureFlags() noexcept;
		[[nodiscard]] static const std::string& getSamplerFlagName(uint64_t flag) noexcept;
		[[nodiscard]] static std::optional<uint64_t> readSamplerFlag(std::string_view name) noexcept;
		[[nodiscard]] static const FlagMap& getSamplerFlags() noexcept;
		[[nodiscard]] static uint64_t getBorderColorFlag(const Color& color) noexcept;

	private:
		bgfx::TextureHandle _handle;
		Config _config;
		static const FlagMap _textureFlags;
		static const FlagMap _samplerFlags;
	};

	class DARMOK_EXPORT BX_NO_VTABLE ITextureLoader : public IFromDefinitionLoader<Texture, Texture::Definition>
	{
	};

	using TextureLoader = FromDefinitionLoader<ITextureLoader, ITextureDefinitionLoader>;

	class DARMOK_EXPORT TextureFileImporter final : public ProtobufFileImporter<ImageTextureDefinitionLoader>
	{
	public:
		TextureFileImporter();
		void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
	private:
		bx::DefaultAllocator _alloc;
		DataLoader _dataLoader;
		ImageLoader _imgLoader;
		ImageTextureDefinitionLoader _defLoader;
	};

}