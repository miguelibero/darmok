#pragma once

#include <darmok/export.h>
#include <darmok/texture_fwd.hpp>
#include <darmok/image.hpp>
#include <darmok/color.hpp>
#include <darmok/glm.hpp>
#include <darmok/data.hpp>
#include <darmok/serialize.hpp>
#include <darmok/loader.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/glm_serialize.hpp>
#include <darmok/utils.hpp>

#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <cereal/cereal.hpp>

#include <string>
#include <memory>

namespace darmok
{
    struct DARMOK_EXPORT TextureConfig final
	{
		glm::uvec2 size = glm::uvec2(1);
		bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8;
		TextureType type = TextureType::Texture2D;
		uint16_t depth = 0;
		bool mips = false;
		uint16_t layers = 1;

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(
				CEREAL_NVP(size),
				CEREAL_NVP(format),
				CEREAL_NVP(type),
				CEREAL_NVP(depth),
				CEREAL_NVP(mips),
				CEREAL_NVP(layers)
			);
		}

		[[nodiscard]] static const TextureConfig& getEmpty() noexcept;

		[[nodiscard]] std::string toString() const noexcept;
		[[nodiscard]] bgfx::TextureInfo getInfo() const noexcept;
	};

	struct DARMOK_EXPORT TextureDefinition final
	{
		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(
				CEREAL_NVP(data),
				CEREAL_NVP(config),
				CEREAL_NVP(flags)
			);
		}

		bool empty() const noexcept;

		Data data;
		TextureConfig config;
		uint64_t flags = defaultTextureLoadFlags;

		static TextureDefinition fromImage(const Image& img, uint64_t flags = defaultTextureLoadFlags) noexcept;
	};

	class DARMOK_EXPORT BX_NO_VTABLE ITextureDefinitionLoader : public ILoader<TextureDefinition>
	{
	};

	class DARMOK_EXPORT CerealTextureDefinitionLoader final : public CerealLoader<ITextureDefinitionLoader>
	{
	public:
		CerealTextureDefinitionLoader(IDataLoader& dataLoader) noexcept;
	};

	class DARMOK_EXPORT ImageTextureDefinitionLoader final : public ITextureDefinitionLoader
	{
	public:
		ImageTextureDefinitionLoader(IImageLoader& imgLoader) noexcept;
		bool supports(const std::filesystem::path& path) const noexcept;
		ImageTextureDefinitionLoader& setLoadFlags(uint64_t flags = defaultTextureLoadFlags) noexcept;
		[[nodiscard]] std::shared_ptr<TextureDefinition> operator()(const std::filesystem::path& path) override;
	private:
		IImageLoader& _imgLoader;
		uint64_t _loadFlags;
	};

	class DARMOK_EXPORT Texture final
	{
	public:
		using Config = TextureConfig;

		Texture(const bgfx::TextureHandle& handle, const Config& cfg) noexcept;
		Texture(const Image& img, uint64_t flags = defaultTextureLoadFlags) noexcept;
		Texture(const Config& cfg, uint64_t flags = defaultTextureLoadFlags) noexcept;
		Texture(const DataView& data, const Config& cfg, uint64_t flags = defaultTextureLoadFlags) noexcept;
		Texture(const TextureDefinition& definition) noexcept;
		Texture(Texture&& other) noexcept;
		Texture& operator=(Texture&& other) noexcept;

		~Texture() noexcept;
		Texture(const Texture& other) = delete;
		Texture& operator=(const Texture& other) = delete;

		void update(const DataView& data, uint8_t mip = 0);
		void update(const DataView& data, const glm::uvec2& size, const glm::uvec2& origin = glm::uvec2(0), uint8_t mip = 0, uint16_t layer = 0, uint8_t side = 0);
		void update(const DataView& data, const glm::uvec3& size, const glm::uvec3& origin = glm::uvec3(0), uint8_t mip = 0);
		uint32_t read(Data& data) noexcept;

		[[nodiscard]] std::string toString() const noexcept;
		[[nodiscard]] const bgfx::TextureHandle& getHandle() const noexcept;
		[[nodiscard]] TextureType getType() const noexcept;
		[[nodiscard]] const glm::uvec2& getSize() const noexcept;
		[[nodiscard]] bgfx::TextureFormat::Enum getFormat() const noexcept;
		[[nodiscard]] uint16_t getLayerCount() const noexcept;
		[[nodiscard]] uint16_t getDepth() const noexcept;
		[[nodiscard]] bool hasMips() const noexcept;
		[[nodiscard]] uint32_t getStorageSize() const noexcept;
		[[nodiscard]] uint8_t getMipsCount() const noexcept;
		[[nodiscard]] uint8_t getBitsPerPixel() const noexcept;

		Texture& setName(std::string_view name) noexcept;

		[[nodiscard]] static const std::string& getFormatName(bgfx::TextureFormat::Enum format) noexcept;
		[[nodiscard]] static std::optional<bgfx::TextureFormat::Enum> readFormat(std::string_view name) noexcept;
		[[nodiscard]] static const std::string& getTypeName(TextureType type) noexcept;
		[[nodiscard]] static std::optional<TextureType> readType(std::string_view name) noexcept;

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
		static const std::array<std::string, bgfx::TextureFormat::Count> _formatNames;
		static const std::array<std::string, toUnderlying(TextureType::Count)> _typeNames;
		static const FlagMap _textureFlags;
		static const FlagMap _samplerFlags;
	};

	class DARMOK_EXPORT BX_NO_VTABLE ITextureLoader : public IFromDefinitionLoader<Texture, TextureDefinition>
	{
	};

	class DARMOK_EXPORT TextureLoader final : public FromDefinitionLoader<ITextureLoader, ITextureDefinitionLoader>
	{
	public:
		TextureLoader(ITextureDefinitionLoader& defLoader) noexcept;
	};

	/*
    class TextureImporterImpl;

	class DARMOK_EXPORT TextureImporter final : public IAssetTypeImporter
	{
	public:
		TextureImporter();
		~TextureImporter() noexcept;

		bool startImport(const Input& input, bool dry = false) override;
		Outputs getOutputs(const Input& input) override;
		Dependencies getDependencies(const Input& input) override;
		void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
		void endImport(const Input& input) override;

		const std::string& getName() const noexcept override;
	private:
		std::unique_ptr<TextureImporterImpl> _impl;
	};
	*/
}