
#pragma once

#include <darmok/image.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/color.hpp>

#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <bx/bx.h>

#include <vector>
#include <string>
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

	typedef glm::vec<2, uint32_t> TextureVec2;

	typedef uint16_t TextureAtlasIndex;

	struct TextureBounds
	{
		TextureVec2 size;
		TextureVec2 offset;
	};

	class Mesh;
	class Material;
	class AnimationFrame;

	struct TextureAtlasElement final
	{
		std::string name;
		std::vector<TextureVec2> positions;
		std::vector<TextureVec2> texCoords;
		std::vector<TextureAtlasIndex> indices;
		TextureVec2 texturePosition;
		TextureVec2 textureSize;
		TextureVec2 originalPosition;
		TextureVec2 originalSize;
		TextureVec2 pivot;
		bool rotated;

		TextureBounds getBounds() const noexcept;
		std::shared_ptr<Mesh> createSprite(const bgfx::VertexLayout& layout, const glm::uvec2& atlasSize, float scale = 1.f) const noexcept;

	};

	struct TextureAtlas final
	{
		std::shared_ptr<Texture> texture;
		std::vector<TextureAtlasElement> elements;
		TextureVec2 size;

		TextureBounds getBounds(std::string_view prefix) const noexcept;
		OptionalRef<TextureAtlasElement> getElement(std::string_view name) noexcept;
		OptionalRef<const TextureAtlasElement> getElement(std::string_view name) const noexcept;
		
		std::shared_ptr<Material> createMaterial(const Color& color = Colors::white) const noexcept;
		std::shared_ptr<Mesh> createSprite(const bgfx::VertexLayout& layout, const TextureAtlasElement& element, float scale = 1.f, const Color& color = Colors::white) const noexcept;
		std::vector<AnimationFrame> createSpriteAnimation(const bgfx::VertexLayout& layout, std::string_view namePrefix, float frameDuration = 1.f / 30.f, float scale = 1.f, const Color& color = Colors::white) const noexcept;

	};

    class BX_NO_VTABLE ITextureLoader
	{
	public:
		
		virtual ~ITextureLoader() = default;
		virtual std::shared_ptr<Texture> operator()(std::string_view name, uint64_t flags = defaultTextureCreationFlags) = 0;
	private:
	};

	class BX_NO_VTABLE ITextureAtlasLoader
	{
	public:
		virtual ~ITextureAtlasLoader() = default;
		virtual std::shared_ptr<TextureAtlas> operator()(std::string_view name, uint64_t flags = defaultTextureCreationFlags) = 0;
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