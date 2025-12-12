#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/color.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/data.hpp>
#include <darmok/loader.hpp>
#include <darmok/image.hpp>
#include <darmok/expected.hpp>
#include <darmok/texture.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/protobuf/mesh.pb.h>
#include <darmok/protobuf/texture_atlas.pb.h>

#include <vector>
#include <string>
#include <memory>
#include <filesystem>

#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <pugixml.hpp>


namespace darmok
{
	struct TextureAtlasBounds final
	{
		glm::uvec2 size;
		glm::uvec2 offset;
	};

	struct TextureAtlasMeshConfig final
	{
		using MeshType = protobuf::Mesh::Type;

		glm::vec3 scale = glm::vec3(1);
		glm::vec3 offset = glm::vec3(0);
		Color color = Colors::white();
		glm::uvec2 amount = glm::uvec2(1);
		MeshType type = protobuf::Mesh::Static;
	};

	struct DARMOK_EXPORT TextureAtlasRmluiConfig final
	{
		std::string nameFormat;
		float resolution = 1.F;
		std::string spriteNameFormat;
		std::string boxNameFormat = "box-*";
	};

	class Mesh;

	namespace TextureAtlasUtils
	{
		using Atlas = protobuf::TextureAtlas;
		using Element = protobuf::TextureAtlasElement;
		using Bounds = TextureAtlasBounds;
		using MeshConfig = TextureAtlasMeshConfig;
		using RmluiConfig = TextureAtlasRmluiConfig;

		Bounds getBounds(const Element& elm) noexcept;
		size_t getVertexAmount(const Element& elm) noexcept;
		bool isRect(const Element& elm) noexcept;

		expected<std::unique_ptr<Mesh>, std::string> createSprite(const Element& elm, const bgfx::VertexLayout& layout, const glm::uvec2& textureSize, const MeshConfig& config = {}) noexcept;
		Element createElement(const Bounds& bounds) noexcept;
		void readTexturePacker(Element& elm, const pugi::xml_node& xml, const glm::uvec2& textureSize) noexcept;
		void writeTexturePacker(const Element& elm, pugi::xml_node& xml) noexcept;

		expected<void, std::string> readTexturePacker(Atlas& atlas, const pugi::xml_document& doc, const std::filesystem::path& basePath = {}) noexcept;
		expected<void, std::string> readTexturePacker(Atlas& atlas, const pugi::xml_node& node, const std::filesystem::path& basePath = {}) noexcept;
		
		struct ImageLoadContext final
		{
			ITextureDefinitionLoader& texLoader;
			bx::AllocatorI& alloc;
		};
		
		expected<void, std::string> writeTexturePacker(const Atlas& atlas, pugi::xml_document& doc, const std::filesystem::path& basePath = {}, const std::optional<ImageLoadContext>& imgLoad = std::nullopt) noexcept;
		expected<void, std::string> writeTexturePacker(const Atlas& atlas, pugi::xml_node& node, const std::filesystem::path& basePath = {}, const std::optional<ImageLoadContext>& imgLoad = std::nullopt) noexcept;
		expected<void, std::string> writeRmlui(const Atlas& atlas, std::ostream& out, const RmluiConfig& config) noexcept;
	}

	class Texture;
	struct AnimationFrame;

	struct DARMOK_EXPORT TextureAtlas final
	{
		using Element = protobuf::TextureAtlasElement;
		using Definition = protobuf::TextureAtlas;

		std::shared_ptr<Texture> texture;
		std::vector<Element> elements;

		TextureAtlasBounds getBounds(std::string_view prefix) const noexcept;
		OptionalRef<Element> getElement(std::string_view name) noexcept;
		OptionalRef<const Element> getElement(std::string_view name) const noexcept;

		using MeshConfig = TextureAtlasMeshConfig;

		expected<std::unique_ptr<Mesh>, std::string> createSprite(std::string_view name, const bgfx::VertexLayout& layout, const MeshConfig& config = {}) const noexcept;
		std::vector<AnimationFrame> createAnimation(const bgfx::VertexLayout& layout, std::string_view namePrefix = "", float frameDuration = 1.f / 30.f, const MeshConfig& config = {}) const noexcept;
	};

	class DARMOK_EXPORT BX_NO_VTABLE ITextureAtlasDefinitionLoader : public ILoader<protobuf::TextureAtlas>{};

	using DataTextureAtlasDefinitionLoader = DataProtobufLoader<ITextureAtlasDefinitionLoader>;

	class DARMOK_EXPORT TexturePackerDefinitionLoader final : public ITextureAtlasDefinitionLoader
	{
	public:
		using Definition = protobuf::TextureAtlas;
		TexturePackerDefinitionLoader(IDataLoader& dataLoader, ITextureDefinitionLoader& texLoader) noexcept;
		Result operator()(std::filesystem::path path) noexcept override;
	private:
		IDataLoader& _dataLoader;
		ITextureDefinitionLoader& _texLoader;
	};

	class DARMOK_EXPORT BX_NO_VTABLE ITextureAtlasLoader : public ILoader<TextureAtlas>{};
	class DARMOK_EXPORT BX_NO_VTABLE ITextureAtlasFromDefinitionLoader : public IFromDefinitionLoader<ITextureAtlasLoader, TextureAtlas::Definition>{};

	class DARMOK_EXPORT TextureAtlasLoader : public FromDefinitionLoader<ITextureAtlasFromDefinitionLoader, ITextureAtlasDefinitionLoader>
	{
	public:
		TextureAtlasLoader(ITextureAtlasDefinitionLoader& defLoader, ITextureLoader& texLoader) noexcept;
	protected:
		Result create(std::shared_ptr<TextureAtlas::Definition> def) noexcept override;
	private:
		ITextureLoader& _texLoader;
	};

	struct TextureAtlasRmluiConfig;

	class DARMOK_EXPORT TexturePackerAtlasFileImporter final : public IFileTypeImporter
	{
	public:
		TexturePackerAtlasFileImporter(std::filesystem::path exePath = "") noexcept;
		const std::string& getName() const noexcept override;
		expected<void, std::string> init(OptionalRef<std::ostream> log) noexcept override;
		expected<Effect, std::string> prepare(const Input& input) noexcept override;
		expected<void, std::string> operator()(const Input& input, Config& config) noexcept override;
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
		std::string _sheetFormat;
		std::filesystem::path _texturePath;
		OptionalRef<std::ostream> _log;
		bx::DefaultAllocator _alloc;

		static std::string getTextureFormatExt(const std::string& format) noexcept;
		static std::string getSheetFormatExt(const std::string& format) noexcept;
		static TextureAtlasRmluiConfig readRmluiConfig(const nlohmann::json& json) noexcept;
	};
}