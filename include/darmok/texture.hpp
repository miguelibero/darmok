
#pragma once

#include <darmok/image.hpp>
#include <darmok/color.hpp>

#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <bx/bx.h>

#include <string>
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

	class BX_NO_VTABLE ITexture
	{
	public:
		virtual ~ITexture() = default;
		virtual bgfx::TextureHandle getHandle() noexcept = 0;
		virtual const bgfx::TextureHandle& getHandle() const noexcept = 0;
		virtual TextureType getType() const noexcept = 0;
	};

    class Texture final : public ITexture
	{
	public:
		Texture(std::shared_ptr<Image> img, uint64_t flags = defaultTextureLoadFlags) noexcept;
		~Texture() noexcept;
		Texture(const Texture& other) = delete;
		Texture& operator=(const Texture& other) = delete;

		bgfx::TextureHandle getHandle() noexcept override;
		const bgfx::TextureHandle& getHandle() const noexcept override;
		TextureType getType() const noexcept override;

		std::shared_ptr<Image> getImage() const noexcept;
		void releaseImage() noexcept;
		Texture& setName(std::string_view name) noexcept;

	private:
		std::shared_ptr<Image> _img;
		uint64_t _flags;
		bgfx::TextureHandle _handle;
		TextureType _type;
		std::string _name;

		void load() noexcept;
	};

	struct RenderTextureConfig final
	{
		glm::uvec2 size;
		uint16_t depth = 0;
		bool mips = false;
		uint16_t layers = 1;
		TextureType type = TextureType::Texture2D;
		bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8;
	};

	class RenderTexture final : public ITexture
	{
	public:
		using Config = RenderTextureConfig;
		RenderTexture(const Config& cfg, uint64_t flags = defaultTextureLoadFlags) noexcept;
		~RenderTexture() noexcept;
		RenderTexture& operator=(const RenderTexture& other) = delete;

		bgfx::TextureHandle getHandle() noexcept override;
		const bgfx::TextureHandle& getHandle() const noexcept override;
		TextureType getType() const noexcept override;

		RenderTexture& setName(std::string_view name) noexcept;
	private:
		bgfx::TextureHandle _handle;
		TextureType _type;
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
		ColorTextureLoader(bx::AllocatorI* alloc, const glm::uvec2& size = { 1, 1 }) noexcept;
		std::shared_ptr<Texture> operator()(const Color& color) noexcept;
	private:
		bx::AllocatorI* _alloc;
		glm::uvec2 _size;
		std::unordered_map<Color, std::shared_ptr<Texture>> _cache;
	};
}