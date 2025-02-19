#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/color.hpp>
#include <darmok/texture_fwd.hpp>
#include <darmok/texture_atlas_fwd.hpp>
#include <darmok/mesh_fwd.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/data.hpp>
#include <darmok/glm_serialize.hpp>
#include <darmok/loader.hpp>

#include <vector>
#include <string>
#include <memory>
#include <filesystem>

#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <pugixml.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>


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

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(
				CEREAL_NVP(scale),
				CEREAL_NVP(offset),
				CEREAL_NVP(color),
				CEREAL_NVP(amount),
				CEREAL_NVP(type)
			);
		}
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
		void readTexturePacker(const pugi::xml_node& xml, const glm::uvec2& textureSize) noexcept;
		void writeTexturePacker(pugi::xml_node& xml) const noexcept;

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(
				CEREAL_NVP(name),
				CEREAL_NVP(positions),
				CEREAL_NVP(texCoords),
				CEREAL_NVP(indices),
				CEREAL_NVP(texturePosition),
				CEREAL_NVP(size),
				CEREAL_NVP(offset),
				CEREAL_NVP(originalSize),
				CEREAL_NVP(pivot),
				CEREAL_NVP(rotated)
			);
		}

	private:
		static std::pair<int, size_t> readInt(std::string_view str, size_t i) noexcept;
		static std::vector<TextureAtlasIndex> readIndexList(std::string_view str) noexcept;
		static std::pair<std::optional<glm::uvec2>, size_t> readVec2(std::string_view str, size_t pos) noexcept;
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

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(
				CEREAL_NVP(nameFormat),
				CEREAL_NVP(resolution),
				CEREAL_NVP(spriteNameFormat),
				CEREAL_NVP(boxNameFormat)
			);
		}
	};

	class ITextureLoader;
	class TextureDefinition;

	struct DARMOK_EXPORT TextureAtlasDefinition final
	{
		std::filesystem::path imagePath;
		std::shared_ptr<TextureDefinition> texture;
		std::vector<TextureAtlasElement> elements;
		glm::uvec2 size{};

		bool readTexturePacker(const pugi::xml_document& doc, const std::filesystem::path& basePath = "");
		bool readTexturePacker(const pugi::xml_node& node, const std::filesystem::path& basePath = "");
		void writeTexturePacker(pugi::xml_document& doc) const noexcept;
		void writeTexturePacker(pugi::xml_node& node) const noexcept;

		using RmluiConfig = TextureAtlasRmluiConfig;
		void writeRmlui(std::ostream& out, const RmluiConfig& config) const noexcept;

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(
				CEREAL_NVP(texture),
				CEREAL_NVP(elements),
				CEREAL_NVP(size)
			);
		}
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

	class DARMOK_EXPORT BX_NO_VTABLE ITextureAtlasDefinitionLoader : public ILoader<TextureAtlasDefinition>
	{
	};

	using CerealTextureAtlasDefinitionLoader = CerealLoader<ITextureAtlasDefinitionLoader>;

	class ITextureLoader;
	class ITextureDefinitionLoader;

	class DARMOK_EXPORT TexturePackerDefinitionLoader final : public ITextureAtlasDefinitionLoader
	{
	public:
		TexturePackerDefinitionLoader(IDataLoader& dataLoader, ITextureDefinitionLoader& texDefLoader) noexcept;
		std::shared_ptr<TextureAtlasDefinition> operator()(std::filesystem::path path) override;
	private:
		IDataLoader& _dataLoader;
		ITextureDefinitionLoader& _texDefLoader;
	};

	class DARMOK_EXPORT BX_NO_VTABLE ITextureAtlasLoader : public IFromDefinitionLoader<TextureAtlas, TextureAtlasDefinition>
	{
	};

	class DARMOK_EXPORT TextureAtlasLoader : public FromDefinitionLoader<ITextureAtlasLoader, ITextureAtlasDefinitionLoader>
	{
	public:
		TextureAtlasLoader(ITextureAtlasDefinitionLoader& defLoader, ITextureLoader& texLoader) noexcept;
	protected:
		std::shared_ptr<TextureAtlas> create(const std::shared_ptr<TextureAtlasDefinition>& def) override;
	private:
		ITextureLoader& _texLoader;
	};

	struct TextureAtlasRmluiConfig;

	class DARMOK_EXPORT TexturePackerAtlasFileImporter final : public IFileTypeImporter
	{
	public:
		TexturePackerAtlasFileImporter(std::filesystem::path exePath = "") noexcept;
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