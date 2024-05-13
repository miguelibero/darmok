
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

		DLLEXPORT Texture(const bgfx::TextureHandle& handle, const Config& cfg) noexcept;
		DLLEXPORT Texture(const Image& img, uint64_t flags = defaultTextureLoadFlags) noexcept;
		DLLEXPORT Texture(const Config& cfg, uint64_t flags = defaultTextureLoadFlags) noexcept;
		DLLEXPORT Texture(const DataView& data, const Config& cfg, uint64_t flags = defaultTextureLoadFlags) noexcept;

		DLLEXPORT ~Texture() noexcept;
		Texture(const Texture& other) = delete;
		Texture& operator=(const Texture& other) = delete;

		DLLEXPORT void update(const DataView& data,uint16_t layer = 0, uint8_t mip = 0);
		DLLEXPORT void update(const DataView& data, const glm::uvec2& size, const glm::uvec2& origin = glm::uvec2(0), uint16_t layer = 0, uint8_t mip = 0);
		DLLEXPORT uint32_t read(Data& data) noexcept;

		DLLEXPORT std::string to_string() const noexcept;
		DLLEXPORT const bgfx::TextureHandle& getHandle() const noexcept;
		DLLEXPORT TextureType getType() const noexcept;
		DLLEXPORT const glm::uvec2& getSize() const noexcept;
		DLLEXPORT bgfx::TextureFormat::Enum getFormat() const noexcept;
		DLLEXPORT uint16_t getLayerCount() const noexcept;
		DLLEXPORT uint16_t getDepth() const noexcept;
		DLLEXPORT bool hasMips() const noexcept;
		DLLEXPORT uint32_t getStorageSize() const noexcept;
		DLLEXPORT uint8_t getMipsCount() const noexcept;
		DLLEXPORT uint8_t getBitsPerPixel() const noexcept;

		Texture& setName(std::string_view name) noexcept;
	private:
		bgfx::TextureHandle _handle;
		Config _config;
	};

    class BX_NO_VTABLE ITextureLoader
	{
	public:
		using result_type = std::shared_ptr<Texture>;
		DLLEXPORT virtual ~ITextureLoader() = default;
		DLLEXPORT virtual result_type operator()(std::string_view name, uint64_t flags = defaultTextureLoadFlags) = 0;
	};

	class ColorTextureLoader final
	{
	public:
		using result_type = std::shared_ptr<Texture>;
		DLLEXPORT ColorTextureLoader(bx::AllocatorI& alloc, const glm::uvec2& size = { 1, 1 }) noexcept;
		DLLEXPORT result_type operator()(const Color& color) noexcept;
	private:
		bx::AllocatorI& _alloc;
		glm::uvec2 _size;
		std::unordered_map<Color, std::shared_ptr<Texture>> _cache;
	};
}