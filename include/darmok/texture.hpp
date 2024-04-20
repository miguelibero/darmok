
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

	using TextureAtlasIndex = uint16_t ;

	struct TextureBounds
	{
		glm::uvec2 size;
		glm::uvec2 offset;
	};

	class Mesh;
	class Material;
	class AnimationFrame;

	struct SpriteCreationConfig final
	{
		glm::vec2 scale = glm::vec2(1);
		glm::vec2 textureScale = glm::vec2(1);
		glm::vec2 offset = glm::vec2(0);
		Color color = Colors::white;
	};

	struct TextureAtlasElement final
	{
		std::string name;
		std::vector<glm::uvec2> positions;
		std::vector<glm::uvec2> texCoords;
		std::vector<TextureAtlasIndex> indices;
		glm::uvec2 texturePosition;
		glm::uvec2 size;
		glm::uvec2 offset;
		glm::uvec2 originalSize;
		glm::vec2 pivot;
		bool rotated;

		TextureBounds getBounds() const noexcept;
		std::shared_ptr<Mesh> createSprite(const bgfx::VertexLayout& layout, const glm::uvec2& atlasSize,
			const SpriteCreationConfig& cfg = {}) const noexcept;
	};

	struct TextureAtlas final
	{
		std::shared_ptr<Texture> texture;
		std::vector<TextureAtlasElement> elements;
		glm::uvec2 size;

		TextureBounds getBounds(std::string_view prefix) const noexcept;
		OptionalRef<TextureAtlasElement> getElement(std::string_view name) noexcept;
		OptionalRef<const TextureAtlasElement> getElement(std::string_view name) const noexcept;

		std::shared_ptr<Material> createMaterial(const Color& color = Colors::white) const noexcept;
		std::shared_ptr<Mesh> createSprite(const bgfx::VertexLayout& layout, const TextureAtlasElement& element, const SpriteCreationConfig& cfg = {}) const noexcept;
		std::vector<AnimationFrame> createSpriteAnimation(const bgfx::VertexLayout& layout, std::string_view namePrefix, float frameDuration = 1.f / 30.f, const SpriteCreationConfig& cfg = {}) const noexcept;

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