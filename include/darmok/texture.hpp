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

	class ConstTextureSourceWrapper
	{
	public:
		using Source = protobuf::TextureSource;
		ConstTextureSourceWrapper(const Source& src) noexcept;

		expected<Image, std::string> createImage(bx::AllocatorI& alloc) noexcept;
	private:
		const Source& _src;
	};

	class ConstTextureDefinitionWrapper
	{
	public:
		using Definition = protobuf::Texture;
		ConstTextureDefinitionWrapper(const Definition& def) noexcept;

		expected<Image, std::string> createImage(bx::AllocatorI& alloc) noexcept;
		expected<void, std::string> writeImage(bx::AllocatorI& alloc, const std::filesystem::path& path) noexcept;

	private:
		const Definition& _def;
	};

	class TextureDefinitionWrapper final : public ConstTextureDefinitionWrapper
	{
	public:
		TextureDefinitionWrapper(Definition& def) noexcept;

		expected<void, std::string> loadSource(const protobuf::TextureSource& src, bx::AllocatorI& alloc) noexcept;
		expected<void, std::string> loadImage(const Image& img) noexcept;
	private:
		Definition& _def;
	};

	class DARMOK_EXPORT BX_NO_VTABLE ITextureDefinitionLoader : public ILoader<protobuf::Texture>{};
	class DARMOK_EXPORT BX_NO_VTABLE ITextureSourceLoader : public ILoader<protobuf::TextureSource>{};

	using DataTextureDefinitionLoader = DataProtobufLoader<ITextureDefinitionLoader>;
	using DataTextureSourceLoader = DataProtobufLoader<ITextureSourceLoader>;

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
		using Source = protobuf::TextureSource;
		using Type = protobuf::Texture::Type;
		using Format = protobuf::Texture::Format;
		using UniformKey = protobuf::TextureUniformKey;

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

		static UniformKey createUniformKey(const std::string& name, uint8_t stage) noexcept;

		[[nodiscard]] expected<void, std::string> update(const DataView& data, uint8_t mip = 0);
		[[nodiscard]] expected<void, std::string> update(const DataView& data, const glm::uvec2& size, const glm::uvec2& origin = glm::uvec2(0), uint8_t mip = 0, uint16_t layer = 0, uint8_t side = 0);
		[[nodiscard]] expected<void, std::string> update(const DataView& data, const glm::uvec3& size, const glm::uvec3& origin = glm::uvec3(0), uint8_t mip = 0);
		[[nodiscard]] uint32_t read(Data& data) noexcept;

		[[nodiscard]] std::string toString() const noexcept;
		[[nodiscard]] const bgfx::TextureHandle& getHandle() const noexcept;
		[[nodiscard]] Type getType() const noexcept;
		[[nodiscard]] glm::uvec2 getSize() const noexcept;
		[[nodiscard]] bgfx::TextureFormat::Enum getFormat() const noexcept;
		[[nodiscard]] uint16_t getLayerCount() const noexcept;
		[[nodiscard]] uint16_t getDepth() const noexcept;
		[[nodiscard]] bool hasMips() const noexcept;
		[[nodiscard]] uint32_t getStorageSize() const noexcept;
		[[nodiscard]] uint8_t getMipsCount() const noexcept;
		[[nodiscard]] uint8_t getBitsPerPixel() const noexcept;

		[[nodiscard]] bgfx::TextureInfo getInfo() const noexcept;
		[[nodiscard]] static const protobuf::TextureConfig& getEmptyConfig() noexcept;
		[[nodiscard]] static Source createSource() noexcept;

		Texture& setName(std::string_view name) noexcept;

		[[nodiscard]] static std::string_view getFormatName(bgfx::TextureFormat::Enum format) noexcept;
		[[nodiscard]] static std::optional<bgfx::TextureFormat::Enum> readFormat(std::string_view name) noexcept;
		[[nodiscard]] static std::string_view getTypeName(Type type) noexcept;
		[[nodiscard]] static std::optional<Type> readType(std::string_view name) noexcept;
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

	class DARMOK_EXPORT BX_NO_VTABLE ITextureLoader : public ILoader<Texture>{};
	class DARMOK_EXPORT BX_NO_VTABLE ITextureFromDefinitionLoader : public IFromDefinitionLoader<ITextureLoader, Texture::Definition>{};
	class DARMOK_EXPORT BX_NO_VTABLE ITextureDefinitionFromSourceLoader : public IFromDefinitionLoader<ITextureDefinitionLoader, Texture::Source>{};

	using TextureLoader = FromDefinitionLoader<ITextureFromDefinitionLoader, ITextureDefinitionLoader>;

	class TextureDefinitionFromSourceLoader final : public FromDefinitionLoader<ITextureDefinitionFromSourceLoader, ITextureSourceLoader>
	{
	public:
		TextureDefinitionFromSourceLoader(ITextureSourceLoader& srcLoader, bx::AllocatorI& alloc) noexcept;
	private:
		Result create(const std::shared_ptr<protobuf::TextureSource>& src) override;
		bx::AllocatorI& _alloc;
	};

	class DARMOK_EXPORT TextureFileImporter final : public ProtobufFileImporter<ImageTextureDefinitionLoader>
	{
	public:
		TextureFileImporter();
		void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
	private:
		bx::DefaultAllocator _alloc;
		FileDataLoader _dataLoader;
		ImageLoader _imgLoader;
		ImageTextureDefinitionLoader _defLoader;
	};

	namespace protobuf
	{
		inline bool operator==(const TextureUniformKey& lhs, const TextureUniformKey& rhs) noexcept
		{
			return lhs.SerializeAsString() == rhs.SerializeAsString();
		}
	}
}

namespace std
{
	template<>
	struct hash<darmok::Texture::UniformKey>
	{
		size_t operator()(const darmok::Texture::UniformKey& key) const noexcept
		{
			return hash<std::string>{}(key.SerializeAsString());
		}
	};
}