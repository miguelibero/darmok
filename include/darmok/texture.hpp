
#pragma once

#include <darmok/image.hpp>
#include <darmok/color.hpp>

#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <bx/bx.h>

#include <string_view>
#include <memory>

namespace darmok
{
	static const uint64_t defaultTextureLoadFlags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE;

	enum class TextureType
	{
		Unknown,
		CubeMap,
		Texture2D,
		Texture3D,
	};

	struct TextureCreationConfig final
	{
		glm::uvec2 size;
		uint16_t depth = 0;
		bool mips = false;
		uint16_t layers = 1;
		TextureType type = TextureType::Texture2D;
		bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8;
	};

    class Texture final
	{
	public:
		Texture(const bgfx::TextureHandle& handle, TextureType type = TextureType::Texture2D, std::shared_ptr<Image> img = nullptr) noexcept;
		~Texture() noexcept;
		Texture(const Texture& other) = delete;
		Texture& operator=(const Texture& other) = delete;

		const bgfx::TextureHandle& getHandle() const noexcept;
		std::shared_ptr<Image> getImage() const noexcept;
		void releaseImage() noexcept;
		TextureType getType() const noexcept;
		Texture& setName(std::string_view name) noexcept;

		static std::shared_ptr<Texture> create(const TextureCreationConfig& cfg, uint64_t flags = defaultTextureLoadFlags) noexcept;
		static std::shared_ptr<Texture> create(std::shared_ptr<Image> img, uint64_t flags = defaultTextureLoadFlags) noexcept;


	private:
		std::shared_ptr<Image> _img;
		bgfx::TextureHandle _handle;
		TextureType _type;
	};

    class BX_NO_VTABLE ITextureLoader
	{
	public:
		
		virtual ~ITextureLoader() = default;
		virtual std::shared_ptr<Texture> operator()(std::string_view name, uint64_t flags = defaultTextureLoadFlags) = 0;
	private:
	};

	class ColorTextureLoader final
	{
	public:
		ColorTextureLoader(bx::AllocatorI* alloc, const glm::uvec2& size = { 1, 1 }) noexcept;
		std::shared_ptr<Texture> operator()(const Color& color) noexcept;
	private:
		bx::AllocatorI* _alloc;
		glm::uvec2 _size;
		std::unordered_map<Color, std::shared_ptr<Texture>> _cache;
	};
}