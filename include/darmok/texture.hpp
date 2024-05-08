
#pragma once


#include <darmok/texture_fwd.hpp>
#include <darmok/image.hpp>
#include <darmok/color.hpp>

#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <bx/bx.h>

#include <string>
#include <memory>

namespace darmok
{
	struct TextureConfig final
	{
		glm::uvec2 size = glm::uvec2(1);
		bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8;
		TextureType type = TextureType::Texture2D;
		uint16_t depth = 0;
		bool mips = false;
		uint16_t layers = 1;

		std::string to_string() const noexcept;
		bgfx::TextureInfo getInfo() const noexcept;
	};

	class Data;
	class DataView;

    class Texture final
	{
	public:
		using Config = TextureConfig;

		Texture(const bgfx::TextureHandle& handle, const Config& cfg) noexcept;
		Texture(const Image& img, uint64_t flags = defaultTextureLoadFlags) noexcept;
		Texture(const Config& cfg, uint64_t flags = defaultTextureLoadFlags) noexcept;
		Texture(const DataView& data, const Config& cfg, uint64_t flags = defaultTextureLoadFlags) noexcept;

		~Texture() noexcept;
		Texture(const Texture& other) = delete;
		Texture& operator=(const Texture& other) = delete;

		void update(const DataView& data,uint16_t layer = 0, uint8_t mip = 0);
		void update(const DataView& data, const glm::uvec2& size, const glm::uvec2& origin = glm::uvec2(0), uint16_t layer = 0, uint8_t mip = 0);
		uint32_t read(Data& data) noexcept;

		std::string to_string() const noexcept;
		const bgfx::TextureHandle& getHandle() const noexcept;
		TextureType getType() const noexcept;
		const glm::uvec2& getSize() const noexcept;
		bgfx::TextureFormat::Enum getFormat() const noexcept;
		uint16_t getLayerCount() const noexcept;
		uint16_t getDepth() const noexcept;
		bool hasMips() const noexcept;
		uint32_t getStorageSize() const noexcept;
		uint8_t getMipsCount() const noexcept;
		uint8_t getBitsPerPixel() const noexcept;

		Texture& setName(std::string_view name) noexcept;
	private:
		bgfx::TextureHandle _handle;
		Config _config;
	};

    class BX_NO_VTABLE ITextureLoader
	{
	public:
		virtual ~ITextureLoader() = default;
		virtual std::shared_ptr<Texture> operator()(std::string_view name, uint64_t flags = defaultTextureLoadFlags) = 0;
	};

	class ColorTextureLoader final
	{
	public:
		ColorTextureLoader(bx::AllocatorI& alloc, const glm::uvec2& size = { 1, 1 }) noexcept;
		std::shared_ptr<Texture> operator()(const Color& color) noexcept;
	private:
		bx::AllocatorI& _alloc;
		glm::uvec2 _size;
		std::unordered_map<Color, std::shared_ptr<Texture>> _cache;
	};
}