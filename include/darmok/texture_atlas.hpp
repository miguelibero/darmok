#pragma once

#include <darmok/export.h>
#include <vector>
#include <string>
#include <memory>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <darmok/glm.hpp>

#include <darmok/optional_ref.hpp>
#include <darmok/color.hpp>
#include <darmok/texture_fwd.hpp>
#include <darmok/texture_atlas_fwd.hpp>
#include <darmok/mesh_fwd.hpp>

namespace darmok
{
	struct TextureAtlasBounds final
	{
		glm::uvec2 size;
		glm::uvec2 offset;
	};

	struct TextureAtlasMeshConfig final
	{
		glm::vec3 scale = glm::vec3(1);
		glm::vec3 offset = glm::vec3(0);
		Color color = Colors::white();
		glm::uvec2 amount = glm::uvec2(1);
		MeshType type = MeshType::Static;
	};

	class IMesh;

	struct DARMOK_EXPORT TextureAtlasElement final
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

		using MeshConfig = TextureAtlasMeshConfig;

		std::unique_ptr<IMesh> createSprite(const bgfx::VertexLayout& layout, const glm::uvec2& textureSize, const MeshConfig& config = {}) const noexcept;

		static TextureAtlasElement create(const TextureAtlasBounds& bounds) noexcept;
	};

	class Texture;

	struct AnimationFrame;

	struct DARMOK_EXPORT TextureAtlas final
	{
		std::shared_ptr<Texture> texture;
		std::vector<TextureAtlasElement> elements;
		glm::uvec2 size;

		TextureAtlasBounds getBounds(std::string_view prefix) const noexcept;
		OptionalRef<TextureAtlasElement> getElement(std::string_view name) noexcept;
		OptionalRef<const TextureAtlasElement> getElement(std::string_view name) const noexcept;

		using MeshConfig = TextureAtlasMeshConfig;

		std::unique_ptr<IMesh> createSprite(std::string_view name, const bgfx::VertexLayout& layout, const MeshConfig& config = {}) const noexcept;
		std::vector<AnimationFrame> createAnimation(const bgfx::VertexLayout& layout, std::string_view namePrefix = "", float frameDuration = 1.f / 30.f, const MeshConfig& config = {}) const noexcept;
	};

	class DARMOK_EXPORT BX_NO_VTABLE ITextureAtlasLoader
	{
	public:
		using result_type = std::shared_ptr<TextureAtlas>;
		virtual ~ITextureAtlasLoader() = default;
		virtual result_type operator()(std::string_view name, uint64_t textureFlags = defaultTextureLoadFlags) = 0;
	};

	class IDataLoader;
	class ITextureLoader;

	class DARMOK_EXPORT TexturePackerTextureAtlasLoader final : public ITextureAtlasLoader
	{
	public:
		TexturePackerTextureAtlasLoader(IDataLoader& dataLoader, ITextureLoader& textureLoader) noexcept;
		std::shared_ptr<TextureAtlas> operator()(std::string_view name, uint64_t textureFlags = defaultTextureLoadFlags) override;
	private:
		IDataLoader& _dataLoader;
		ITextureLoader& _textureLoader;
	};
}