#pragma once

#include <vector>
#include <string>
#include <memory>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

#include <darmok/optional_ref.hpp>
#include <darmok/color.hpp>
#include <darmok/texture_fwd.hpp>
#include <darmok/texture_atlas_fwd.hpp>
#include <darmok/material_fwd.hpp>
#include <darmok/mesh_fwd.hpp>

namespace darmok
{
	struct TextureAtlasBounds final
	{
		glm::uvec2 size;
		glm::uvec2 offset;
	};

	struct TextureAtlasMeshCreationConfig final
	{
		glm::vec3 scale = glm::vec3(1);
		glm::vec3 offset = glm::vec3(0);
		Color color = Colors::white();
		glm::uvec2 amount = glm::uvec2(1);
		MeshType type = MeshType::Static;
	};

	class IMesh;

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

		DLLEXPORT TextureAtlasBounds getBounds() const noexcept;
		DLLEXPORT size_t getVertexAmount() const noexcept;

		using Config = TextureAtlasMeshCreationConfig;

		DLLEXPORT std::shared_ptr<IMesh> createSprite(const bgfx::VertexLayout& layout, const glm::uvec2& textureSize, Config config) const noexcept;

		DLLEXPORT static TextureAtlasElement create(const TextureAtlasBounds& bounds) noexcept;
	};

	class Texture;

	struct TextureAtlas final
	{
		std::shared_ptr<Texture> texture;
		std::vector<TextureAtlasElement> elements;
		glm::uvec2 size;

		DLLEXPORT TextureAtlasBounds getBounds(std::string_view prefix) const noexcept;
		DLLEXPORT OptionalRef<TextureAtlasElement> getElement(std::string_view name) noexcept;
		DLLEXPORT OptionalRef<const TextureAtlasElement> getElement(std::string_view name) const noexcept;
	};

	struct AnimationFrame;

	struct TextureAtlasMeshCreator final
	{
		using Config = TextureAtlasMeshCreationConfig;
		bgfx::VertexLayout layout;
		const TextureAtlas& atlas;
		Config config;

		DLLEXPORT TextureAtlasMeshCreator(const bgfx::VertexLayout& layout, const TextureAtlas& atlas) noexcept;

		DLLEXPORT std::shared_ptr<IMesh> createSprite(std::string_view name) const noexcept;
		DLLEXPORT std::vector<AnimationFrame> createAnimation(std::string_view namePrefix, float frameDuration = 1.f / 30.f) const noexcept;
	};

    class BX_NO_VTABLE ITextureAtlasLoader
	{
	public:
		using result_type = std::shared_ptr<TextureAtlas>;
		DLLEXPORT virtual ~ITextureAtlasLoader() = default;
		DLLEXPORT virtual result_type operator()(std::string_view name, uint64_t textureFlags = defaultTextureLoadFlags) = 0;
	};
}