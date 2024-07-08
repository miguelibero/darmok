#pragma once

#include <darmok/export.h>
#include <vector>
#include <string>
#include <memory>
#include <filesystem>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <pugixml.hpp>

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
		std::string name = "";
		std::vector<glm::uvec2> positions;
		std::vector<glm::uvec2> texCoords;
		std::vector<TextureAtlasIndex> indices;
		glm::uvec2 texturePosition = {};
		glm::uvec2 size = {};
		glm::uvec2 offset = {};
		glm::uvec2 originalSize = {};
		glm::vec2 pivot = {};
		bool rotated = false;

		TextureAtlasBounds getBounds() const noexcept;
		size_t getVertexAmount() const noexcept;
		bool isRect() const noexcept;

		using MeshConfig = TextureAtlasMeshConfig;

		std::unique_ptr<IMesh> createSprite(const bgfx::VertexLayout& layout, const glm::uvec2& textureSize, const MeshConfig& config = {}) const noexcept;
		static TextureAtlasElement create(const TextureAtlasBounds& bounds) noexcept;
		void read(const pugi::xml_node& xml, const glm::uvec2& textureSize) noexcept;
		void write(pugi::xml_node& xml) const noexcept;

	private:
		static std::pair<int, size_t> readInt(std::string_view str, size_t i) noexcept;
		static std::vector<TextureAtlasIndex> readIndexList(std::string_view str) noexcept;
		static std::pair<std::optional<glm::uvec2>, size_t> readVec2(std::string_view str, size_t i) noexcept;
		static std::vector<glm::uvec2> readVec2List(std::string_view data) noexcept;
		static std::string writeVec2List(const std::vector<glm::uvec2>& list) noexcept;
		static std::string writeIndexList(const std::vector<TextureAtlasIndex>& list) noexcept;
	};

	class ImageAtlas;

	struct DARMOK_EXPORT TextureAtlasData final
	{
		std::filesystem::path imagePath;
		std::vector<TextureAtlasElement> elements;
		glm::uvec2 size;

		bool read(const pugi::xml_document& doc, const std::filesystem::path& basePath = "");
		bool read(const pugi::xml_node& node, const std::filesystem::path& basePath = "");
		void write(pugi::xml_document& doc) const noexcept;
		void write(pugi::xml_node& node) const noexcept;
	};

	class Texture;
	struct AnimationFrame;

	struct DARMOK_EXPORT TextureAtlas final
	{
		std::shared_ptr<Texture> texture;
		std::vector<TextureAtlasElement> elements;

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