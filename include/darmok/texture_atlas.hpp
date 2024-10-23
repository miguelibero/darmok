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
#include <darmok/asset_core.hpp>
#include <darmok/data.hpp>

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
		glm::vec2 offset = {};
		glm::vec2 originalSize = {};
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

	struct DARMOK_EXPORT TextureAtlasRmluiConfig final
	{
		std::string nameFormat;
		float resolution = 1.F;
		std::string spriteNameFormat;
		std::string boxNameFormat = "box-*";
	};

	struct DARMOK_EXPORT TextureAtlasData final
	{
		std::filesystem::path imagePath;
		std::vector<TextureAtlasElement> elements;
		glm::uvec2 size;

		bool read(const pugi::xml_document& doc, const std::filesystem::path& basePath = "");
		bool read(const pugi::xml_node& node, const std::filesystem::path& basePath = "");
		void write(pugi::xml_document& doc) const noexcept;
		void write(pugi::xml_node& node) const noexcept;

		using RmluiConfig = TextureAtlasRmluiConfig;
		void writeRmlui(std::ostream& out, const RmluiConfig& config) const noexcept;
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

	struct TextureAtlasRmluiConfig;

	class DARMOK_EXPORT TexturePackerTextureAtlasImporter final : public IAssetTypeImporter
	{
	public:
		TexturePackerTextureAtlasImporter(const std::filesystem::path& exePath = "") noexcept;
		void setLogOutput(OptionalRef<std::ostream> log) noexcept override;
		bool startImport(const Input& input, bool dry) override;
		Outputs getOutputs(const Input& input) override;
		Dependencies getDependencies(const Input& input) override;
		void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
		const std::string& getName() const noexcept override;
		void endImport(const Input& input) override;
	private:
		static const std::unordered_map<std::string, std::string> _textureFormatExts;
		static const std::unordered_map<std::string, std::string> _sheetFormatExts;
		static const std::string _outputPathOption;
		static const std::string _outputFormatOption;
		static const std::string _textureFormatOption;
		static const std::string _rmluiResolutionOption;
		static const std::string _rmluiNameFormatOption;
		static const std::string _rmluiSpriteNameFormatOption;
		static const std::string _rmluiBoxNameFormatOption;
		std::filesystem::path _exePath;
		pugi::xml_document _xmlDoc;
		std::filesystem::path _texturePath;
		std::filesystem::path _sheetPath;
		Data _textureData;
		Data _sheetData;
		OptionalRef<std::ostream> _log;

		static std::string getTextureFormatExt(const std::string& format) noexcept;
		static std::string getSheetFormatExt(const std::string& format) noexcept;
		static TextureAtlasRmluiConfig readRmluiConfig(const nlohmann::json& json) noexcept;
	};
}