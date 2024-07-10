
#pragma once

#include <darmok/export.h>
#include <darmok/texture_fwd.hpp>
#include <darmok/image.hpp>
#include <darmok/color.hpp>

#include <bgfx/bgfx.h>
#include <darmok/glm.hpp>
#include <bx/bx.h>

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
			archive(size, format, type, depth, mips, layers);
		}

		static [[nodiscard]] const TextureConfig& getEmpty() noexcept;

		[[nodiscard]] std::string to_string() const noexcept;
		[[nodiscard]] bgfx::TextureInfo getInfo() const noexcept;
	};

	class Data;
	class DataView;

	class DARMOK_EXPORT Texture final
	{
	public:
		using Config = TextureConfig;

		// TODO: remove bgfx flags param and move options to texture config struct

		Texture(const bgfx::TextureHandle& handle, const Config& cfg) noexcept;
		Texture(const Image& img, uint64_t flags = defaultTextureLoadFlags) noexcept;
		Texture(const Config& cfg, uint64_t flags = defaultTextureLoadFlags) noexcept;
		Texture(const DataView& data, const Config& cfg, uint64_t flags = defaultTextureLoadFlags) noexcept;
		Texture(Texture&& other) noexcept;
		Texture& operator=(Texture&& other) noexcept;

		~Texture() noexcept;
		Texture(const Texture& other) = delete;
		Texture& operator=(const Texture& other) = delete;

		void update(const DataView& data, uint8_t mip = 0);
		void update(const DataView& data, const glm::uvec2& size, const glm::uvec2& origin = glm::uvec2(0), uint8_t mip = 0, uint16_t layer = 0, uint8_t side = 0);
		void update(const DataView& data, const glm::uvec3& size, const glm::uvec3& origin = glm::uvec3(0), uint8_t mip = 0);
		uint32_t read(Data& data) noexcept;

		[[nodiscard]] std::string to_string() const noexcept;
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

	private:
		bgfx::TextureHandle _handle;
		Config _config;
	};

	class DARMOK_EXPORT BX_NO_VTABLE ITextureLoader
	{
	public:
		using result_type = std::shared_ptr<Texture>;
		virtual ~ITextureLoader() = default;
		virtual [[nodiscard]] result_type operator()(std::string_view name, uint64_t flags = defaultTextureLoadFlags) = 0;
	};

	class IImageLoader;

	class DARMOK_EXPORT ImageTextureLoader final : public ITextureLoader
	{
	public:
		ImageTextureLoader(IImageLoader& imgLoader) noexcept;
		[[nodiscard]] result_type operator()(std::string_view name, uint64_t flags = defaultTextureLoadFlags) noexcept override;
	private:
		IImageLoader& _imgLoader;
	};

	class DARMOK_EXPORT ColorTextureLoader final
	{
	public:
		using result_type = std::shared_ptr<Texture>;
		ColorTextureLoader(bx::AllocatorI& alloc, const glm::uvec2& size = { 1, 1 }) noexcept;
		[[nodiscard]] result_type operator()(const Color& color) noexcept;
	private:
		bx::AllocatorI& _alloc;
		glm::uvec2 _size;
		std::unordered_map<Color, std::shared_ptr<Texture>> _cache;
	};
}