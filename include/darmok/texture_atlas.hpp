#pragma once

#include <vector>
#include <string>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

#include <darmok/optional_ref.hpp>

namespace darmok
{
    using TextureAtlasIndex = uint16_t ;

	struct TextureAtlasBounds
	{
		glm::uvec2 size;
		glm::uvec2 offset;
	};

	class Texture;
	class Material;
	struct AnimationFrame;

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

		TextureAtlasBounds getBounds() const noexcept;
		size_t getVertexAmount() const noexcept;
	};

	struct TextureAtlas final
	{
		std::shared_ptr<Texture> texture;
		std::vector<TextureAtlasElement> elements;
		glm::uvec2 size;

		TextureAtlasBounds getBounds(std::string_view prefix) const noexcept;
		OptionalRef<TextureAtlasElement> getElement(std::string_view name) noexcept;
		OptionalRef<const TextureAtlasElement> getElement(std::string_view name) const noexcept;
	};

	struct TextureAtlasMeshCreationConfig final
	{
		glm::vec3 scale = glm::vec3(1);
		glm::vec3 offset = glm::vec3(0);
		Color color = Colors::white();
	};

	struct TextureAtlasMeshCreator final
	{
		using Config = TextureAtlasMeshCreationConfig;
		bgfx::VertexLayout layout;
		const TextureAtlas& atlas;
		Config config;

		TextureAtlasMeshCreator(const bgfx::VertexLayout& layout, const TextureAtlas& atlas, const Config& cfg = {}) noexcept;

		std::shared_ptr<Mesh> createSprite(std::string_view name) const noexcept;
		std::vector<AnimationFrame> createAnimation(std::string_view namePrefix, float frameDuration = 1.f / 30.f) const noexcept;
	private:


		std::shared_ptr<Mesh> createSprite(const TextureAtlasElement& elm) const noexcept;
		std::shared_ptr<Material> createMaterial(const Color& color = Colors::white()) const noexcept;
	};

    class BX_NO_VTABLE ITextureAtlasLoader
	{
	public:
		virtual ~ITextureAtlasLoader() = default;
		virtual std::shared_ptr<TextureAtlas> operator()(std::string_view name, uint64_t flags = defaultTextureCreationFlags) = 0;
	};
}