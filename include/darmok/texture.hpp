
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
	static const uint64_t defaultTextureCreationFlags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE;

	enum class TextureType
	{
		Unknown,
		CubeMap,
		Texture2D,
		Texture3D,
	};

    class Texture final
	{
	public:
		Texture(std::shared_ptr<Image> img, const bgfx::TextureHandle& handle, TextureType type = TextureType::Texture2D) noexcept;
		~Texture() noexcept;
		Texture(const Texture& other) = delete;
		Texture& operator=(const Texture& other) = delete;

		const bgfx::TextureHandle& getHandle() const noexcept;
		std::shared_ptr<Image> getImage() const noexcept;
		void releaseImage() noexcept;
		TextureType getType() const noexcept;

		static std::shared_ptr<Texture> create(std::shared_ptr<Image> img, std::string_view name = "", uint64_t flags = defaultTextureCreationFlags) noexcept;


	private:
		std::shared_ptr<Image> _img;
		bgfx::TextureHandle _handle;
		TextureType _type;
	};

    class BX_NO_VTABLE ITextureLoader
	{
	public:
		
		virtual ~ITextureLoader() = default;
		virtual std::shared_ptr<Texture> operator()(std::string_view name, uint64_t flags = defaultTextureCreationFlags) = 0;
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