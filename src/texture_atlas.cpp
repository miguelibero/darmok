#include <darmok/texture_atlas.hpp>
#include <darmok/data.hpp>
#include <darmok/mesh.hpp>
#include <darmok/anim.hpp>
#include <darmok/vertex.hpp>
#include <darmok/texture.hpp>
#include <darmok/string.hpp>
#include <darmok/shape.hpp>
#include <darmok/utils.hpp>
#include <darmok/data_stream.hpp>
#include <bx/platform.h>

#include <filesystem>
#include <charconv>
#include <pugixml.hpp>

namespace darmok
{
    TextureAtlasBounds TextureAtlasElement::getBounds() const noexcept
	{
		return {
			originalSize,
			offset
		};
	}

	size_t TextureAtlasElement::getVertexAmount() const noexcept
	{
		return std::min(positions.size(), texCoords.size());
	}

	TextureAtlasElement TextureAtlasElement::create(const TextureAtlasBounds& bounds) noexcept
	{
		static const std::vector<glm::uvec2> elms = { { 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 } };
		static const std::vector<VertexIndex> idxs = { 0, 1, 2, 2, 3, 0 };
		TextureAtlasElement elm{ "", elms, elms, idxs, bounds.offset, bounds.size, {}, bounds.size, {}, false };

		for (auto& pos : elm.positions)
		{
			pos = pos * bounds.size;
		}
		for (auto& texCoord : elm.texCoords)
		{
			texCoord.y = texCoord.y ? 0 : 1;
			texCoord = bounds.offset + texCoord * bounds.size;
		}

		return elm;
	}

	std::unique_ptr<IMesh> TextureAtlasElement::createSprite(const bgfx::VertexLayout& layout, const glm::uvec2& textureSize, const MeshConfig& config) const noexcept
	{
		auto vertexAmount = uint32_t(getVertexAmount());
		VertexDataWriter writer(layout, vertexAmount * config.amount.x * config.amount.y);
		std::vector<VertexIndex> totalIndices;
		totalIndices.reserve(indices.size() * config.amount.x * config.amount.y);

		glm::vec2 fatlasSize(textureSize);

		auto baseOffset = config.offset - glm::vec3(pivot * glm::vec2(originalSize), 0);
		// don't think this is needed since it's already factored into the position values
		// baseOffset += glm::vec3(offset, 0);
		auto amountStep = glm::vec2(originalSize);
		auto amountOffsetMax = (glm::vec2(config.amount) - glm::vec2(1)) * amountStep * 0.5F;
		auto amountOffset = -glm::vec3(amountOffsetMax, 0.F);

		uint32_t vertexIndex = 0;
		for (; amountOffset.x <= amountOffsetMax.x; amountOffset.x += amountStep.x)
		{
			amountOffset.y = -amountOffsetMax.y;
			for (; amountOffset.y <= amountOffsetMax.y; amountOffset.y += amountStep.y)
			{
				auto elmOffset = baseOffset + amountOffset;
				for (uint32_t i = 0; i < vertexAmount; i++)
				{
					auto& texPos = positions[i];
					auto pos = (elmOffset + glm::vec3(texPos.x, float(originalSize.y) - texPos.y, 0)) * config.scale;
					writer.write(bgfx::Attrib::Position, vertexIndex + i, pos);
					auto texCoord = glm::vec2(texCoords[i]) / fatlasSize;
					writer.write(bgfx::Attrib::TexCoord0, vertexIndex + i, texCoord);
				}
				for (auto& idx : indices)
				{
					totalIndices.push_back(vertexIndex + idx);
				}
				vertexIndex += vertexAmount;
			}
		}

		if (layout.has(bgfx::Attrib::Normal))
		{
			static const glm::vec3 norm(0, 0, -1);
			writer.write(bgfx::Attrib::Normal, norm);
		}
		if (layout.has(bgfx::Attrib::Color0))
		{
			writer.write(bgfx::Attrib::Color0, config.color);
		}

		Data vertexData = writer.finish();
		return IMesh::create(config.type, layout, DataView(vertexData), DataView(totalIndices));
	}

	std::pair<int, size_t> TextureAtlasElement::readInt(std::string_view str, size_t i) noexcept
	{
		if (str.size() == 0 || i == std::string::npos || i >= str.size())
		{
			return { -1, std::string::npos };
		}
		auto ni = str.find(' ', i);
		if (ni == std::string::npos)
		{
			ni = str.size();
		}
		int v;
		std::from_chars(str.data() + i, str.data() + ni, v);
		return { v, ni + 1 };
	}

	std::vector<TextureAtlasIndex> TextureAtlasElement::readIndexList(std::string_view str) noexcept
	{
		std::vector<TextureAtlasIndex> list;
		size_t ii = 0;
		while (ii != std::string::npos)
		{
			auto p = readInt(str, ii);
			if (p.first < 0)
			{
				break;
			}
			list.push_back(p.first);
			ii = p.second;
		}
		return list;
	}

	std::pair<std::optional<glm::uvec2>, size_t> TextureAtlasElement::readVec2(std::string_view str, size_t i) noexcept
	{
		auto p = readInt(str, i);
		if (p.first < 0)
		{
			return { std::nullopt, std::string::npos };
		}
		glm::uvec2 v = { p.first, 0 };
		i = p.second;
		if (i == std::string::npos)
		{
			return { v, i };
		}
		p = readInt(str, i);
		if (p.first < 0)
		{
			return { std::nullopt, std::string::npos };
		}
		v.y = p.first;
		i = p.second;
		return { v, i };
	}

	std::vector<glm::uvec2> TextureAtlasElement::readVec2List(std::string_view data) noexcept
	{
		std::vector<glm::uvec2> list;
		size_t pi = 0;
		while (pi != std::string::npos)
		{
			auto pp = readVec2(data, pi);
			if (!pp.first.has_value())
			{
				break;
			}
			list.push_back(pp.first.value());
			pi = pp.second;
		}
		return list;
	}

	bool TextureAtlasElement::isRect() const noexcept
	{
		if (positions.size() != 4)
		{
			return false;
		}
		if (positions[0] != glm::uvec2(size.x, 0))
		{
			return false;
		}
		if (positions[1] != size)
		{
			return false;
		}
		if (positions[2] != glm::uvec2(0, size.y))
		{
			return false;
		}
		if (positions[3] != glm::uvec2(0))
		{
			return false;
		}
		return true;
	}

	void TextureAtlasElement::readTexturePacker(const pugi::xml_node& xml, const glm::uvec2& textureSize) noexcept
	{
		name = xml.attribute("n").value();

		texturePosition = glm::uvec2(0);
		size = textureSize;

		auto xmlX = xml.attribute("x");
		if (xmlX)
		{
			texturePosition.x = xmlX.as_int();
		}
		auto xmlY = xml.attribute("y");
		if (xmlY)
		{
			texturePosition.y = xmlY.as_int();
		}
		auto xmlW = xml.attribute("w");
		if (xmlW)
		{
			size.x = xmlW.as_int();
		}
		auto xmlH = xml.attribute("h");
		if (xmlH)
		{
			size.y = xmlH.as_int();
		}

		offset = glm::uvec2(0);
		auto xmlOriginalX = xml.attribute("oX");
		if (xmlOriginalX)
		{
			offset.x = xmlOriginalX.as_float();
		}
		auto xmlOriginalY = xml.attribute("oY");
		if (xmlOriginalY)
		{
			offset.y = xmlOriginalY.as_float();
		}

		auto xmlVertices = xml.child("vertices");
		if (xmlVertices)
		{
			positions = readVec2List(xmlVertices.text().get());
		}
		else
		{
			positions = {
				glm::uvec2(size.x, 0), size,
				glm::uvec2(0, size.y), glm::uvec2(0),
			};
		}

		auto xmlVerticesUV = xml.child("verticesUV");
		if (xmlVerticesUV)
		{
			texCoords = readVec2List(xmlVerticesUV.text().get());
		}
		else
		{
			texCoords.reserve(positions.size());
			for (auto& p : positions)
			{
				texCoords.push_back(texturePosition + p);
			}
		}

		auto xmlTriangles = xml.child("triangles");
		if (xmlTriangles)
		{
			indices = readIndexList(xmlTriangles.text().get());
		}
		else
		{
			indices = { 0, 1, 2, 2, 3, 0 };
		}

		originalSize = size;
		auto xmlOriginalW = xml.attribute("oW");
		if (xmlOriginalW)
		{
			originalSize.x = xmlOriginalW.as_int();
		}
		auto xmlOriginalH = xml.attribute("oH");
		if (xmlOriginalH)
		{
			originalSize.y = xmlOriginalH.as_int();
		}

		pivot = { xml.attribute("pX").as_float(), xml.attribute("pY").as_float() };
		rotated = std::string(xml.attribute("r").value()) == "y";
	}

	void TextureAtlasElement::writeTexturePacker(pugi::xml_node& xml) const noexcept
	{
		xml.append_attribute("n") = name.c_str();
		xml.append_attribute("x") = texturePosition.x;
		xml.append_attribute("y") = texturePosition.y;
		xml.append_attribute("w") = size.x;
		xml.append_attribute("h") = size.y;
		if (offset.x != 0)
		{
			xml.append_attribute("oX") = offset.x;
		}
		if (offset.y != 0)
		{
			xml.append_attribute("oY") = offset.y;
		}
		if (originalSize.x != 0)
		{
			xml.append_attribute("oW") = originalSize.x;
		}
		if (originalSize.y != 0)
		{
			xml.append_attribute("oH") = originalSize.y;
		}
		if (pivot.x != 0)
		{
			xml.append_attribute("pX") = pivot.x;
		}
		if (pivot.y != 0)
		{
			xml.append_attribute("pY") = pivot.y;
		}
		if (rotated)
		{
			xml.append_attribute("r") = "y";
		}
		if (!isRect())
		{
			if (!positions.empty())
			{
				xml.append_child("vertices").set_value(writeVec2List(positions).c_str());
			}
			if (!texCoords.empty())
			{
				xml.append_child("verticesUV").set_value(writeVec2List(texCoords).c_str());
			}
			if (!indices.empty())
			{
				xml.append_child("triangles").set_value(writeIndexList(indices).c_str());
			}
		}
	}

	std::string TextureAtlasElement::writeVec2List(const std::vector<glm::uvec2>& list) noexcept
	{
		return StringUtils::join(" ", list, [](auto& v) {
			return std::to_string(v.x) + " " + std::to_string(v.y);
		});
	}

	std::string TextureAtlasElement::writeIndexList(const std::vector<TextureAtlasIndex>& list) noexcept
	{
		return StringUtils::join(" ", list, [](auto& v) {
			return std::to_string(v);
		});
	}

	TextureAtlasBounds TextureAtlas::getBounds(std::string_view prefix) const noexcept
	{
		TextureAtlasBounds bounds{};
		for (auto& elm : elements)
		{
			if (StringUtils::startsWith(elm.name, prefix))
			{
				// TODO: algorithm to combine bounds
				bounds = elm.getBounds();
			}
		}
		return bounds;
	}

	OptionalRef<TextureAtlasElement> TextureAtlas::getElement(std::string_view name) noexcept
	{
		for (auto& elm : elements)
		{
			if (elm.name == name)
			{
				return elm;
			}
		}
		return nullptr;
	}

	OptionalRef<const TextureAtlasElement> TextureAtlas::getElement(std::string_view name) const noexcept
	{
		for (auto& elm : elements)
		{
			if (elm.name == name)
			{
				return elm;
			}
		}
		return nullptr;
	}

	std::unique_ptr<IMesh> TextureAtlas::createSprite(std::string_view name, const bgfx::VertexLayout& layout, const MeshConfig& config) const noexcept
	{
		auto elm = getElement(name);
		if (!elm)
		{
			return nullptr;
		}
		auto& size = texture->getSize();
		return elm->createSprite(layout, size, config);
	}

	std::vector<AnimationFrame> TextureAtlas::createAnimation(const bgfx::VertexLayout& layout, std::string_view namePrefix, float frameDuration, const MeshConfig& config) const noexcept
	{
		std::vector<AnimationFrame> frames;
		auto& size = texture->getSize();

		for (auto& elm : elements)
		{
			if (StringUtils::startsWith(elm.name, namePrefix))
			{
				auto mesh = elm.createSprite(layout, size, config);
				if (mesh)
				{
					frames.emplace_back(std::move(mesh), frameDuration);
				}
			}
		}
		return frames;
	}

	bool TextureAtlasDefinition::readTexturePacker(const pugi::xml_document& doc, const std::filesystem::path& basePath)
	{
		if (doc.empty())
		{
			return false;
		}
		return readTexturePacker(doc.child("TextureAtlas"), basePath);
	}

	bool TextureAtlasDefinition::readTexturePacker(const pugi::xml_node& atlasXml, const std::filesystem::path& basePath)
	{
		if (atlasXml.empty())
		{
			return false;
		}

		imagePath = basePath / std::filesystem::path(atlasXml.attribute("imagePath").value());

		size = glm::uvec2{
			atlasXml.attribute("width").as_int(),
			atlasXml.attribute("height").as_int(),
		};

		static const char* spriteTag = "sprite";
		for (pugi::xml_node spriteXml = atlasXml.child(spriteTag); spriteXml; spriteXml = spriteXml.next_sibling(spriteTag))
		{
			elements.emplace_back().readTexturePacker(spriteXml, size);
		}

		return true;
	}

	void TextureAtlasDefinition::writeTexturePacker(pugi::xml_document& doc) const noexcept
	{
		pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
		decl.append_attribute("version") = "1.0";
		decl.append_attribute("encoding") = "utf-8";
		auto node = doc.append_child("TextureAtlas");
		writeTexturePacker(node);
	}

	void TextureAtlasDefinition::writeTexturePacker(pugi::xml_node& xml) const noexcept
	{
		xml.append_attribute("imagePath") = imagePath.string().c_str();
		for (auto& elm : elements)
		{
			auto sprite = xml.append_child("sprite");
			elm.writeTexturePacker(sprite);
		}
	}

	void TextureAtlasDefinition::writeRmlui(std::ostream& out, const RmluiConfig& config) const noexcept
	{
		auto name = imagePath.stem().string();
		if (!config.nameFormat.empty())
		{
			auto origName = name;
			name = config.nameFormat;
			StringUtils::replace(name, "*", origName);
		}
		out << "@spritesheet " << name << std::endl;
		out << "{" << std::endl;
		out << "    src: " << imagePath.string() << ";" << std::endl;
		out << "    resolution: " << config.resolution << "x;" << std::endl;

		for (auto& elm : elements)
		{
			glm::uvec4 v(elm.texturePosition, elm.size);
			auto name = StringUtils::getFileStem(elm.name);
			if (!config.spriteNameFormat.empty())
			{
				auto origName = name;
				name = config.spriteNameFormat;
				StringUtils::replace(name, "*", origName);
			}
			out << "    " << name << ": " << v.x << "px " << v.y << "px " << v.z << "px " << v.w << "px;" << std::endl;
		}
		out << "}" << std::endl;

		if (!config.boxNameFormat.empty())
		{
			for (auto& elm : elements)
			{
				out << std::endl;
				auto name = config.boxNameFormat;
				auto origName = StringUtils::getFileStem(elm.name);
				StringUtils::replace(name, "*", origName);

				out << "." << name << " {" << std::endl;
				out << "  width: " << elm.originalSize.x << "px;" << std::endl;
				out << "  height: " << elm.originalSize.y << "px;" << std::endl;
				out << "  padding-left: " << elm.offset.x << "px;" << std::endl;
				out << "  padding-bottom: " << elm.offset.y << "px;" << std::endl;
				out << "}" << std::endl;
			}
		}
	}

	CerealTextureAtlasDefinitionLoader::CerealTextureAtlasDefinitionLoader(IDataLoader& dataLoader) noexcept
		: CerealLoader(dataLoader)
	{
	}

	TexturePackerDefinitionLoader::TexturePackerDefinitionLoader(IDataLoader& dataLoader, ITextureDefinitionLoader& texDefLoader) noexcept
		: _dataLoader(dataLoader)
		, _texDefLoader(texDefLoader)
	{
	}

	std::shared_ptr<TextureAtlasDefinition> TexturePackerDefinitionLoader::operator()(const std::filesystem::path& path)
	{
		auto data = _dataLoader(path);

		pugi::xml_document doc;
		auto result = doc.load_buffer_inplace(data.ptr(), data.size());
		if (result.status != pugi::status_ok)
		{
			throw std::runtime_error(result.description());
		}
		auto atlasDef = std::make_shared<TextureAtlasDefinition>();
		if (!atlasDef->readTexturePacker(doc, path.parent_path()))
		{
			throw std::runtime_error("failed to read texture packer xml");
		}
		atlasDef->texture = _texDefLoader(atlasDef->imagePath);
		return atlasDef;
	}

	TextureAtlasLoader::TextureAtlasLoader(ITextureAtlasDefinitionLoader& defLoader, ITextureLoader& texLoader) noexcept
		: FromDefinitionLoader(defLoader)
		, _texLoader(texLoader)
	{
	}

	std::shared_ptr<TextureAtlas> TextureAtlasLoader::create(const std::shared_ptr<TextureAtlasDefinition>& def)
	{
		auto atlas = std::make_shared<TextureAtlas>();
		atlas->elements = def->elements;
		if (def->texture)
		{
			atlas->texture = _texLoader.loadResource(def->texture);
		}
		else
		{
			atlas->texture = _texLoader(def->imagePath);
		}
		return atlas;
	}

	TexturePackerTextureAtlasImporter::TexturePackerTextureAtlasImporter(const std::filesystem::path& exePath) noexcept
		: _exePath(exePath)
	{
		if (_exePath.empty())
		{
			_exePath = "TexturePacker";
		}
#if BX_PLATFORM_WINDOWS
		_exePath += ".exe";
#endif
	}

	void TexturePackerTextureAtlasImporter::setLogOutput(OptionalRef<std::ostream> log) noexcept
	{
		_log = log;
	}

	const std::string& TexturePackerTextureAtlasImporter::getName() const noexcept
	{
		static const std::string name("texture_packer");
		return name;
	}

	const std::unordered_map<std::string, std::string> TexturePackerTextureAtlasImporter::_textureFormatExts = {
		{ "png8", ".png"},
		{ "pvr3", ".pvr"},
		{ "pvr3gz", ".pvr.gz"},
		{ "pvr3ccz", ".pvr.ccz"},
		{ "pvr3ccz", ".pvr.ccz"},
	};

	std::string TexturePackerTextureAtlasImporter::getTextureFormatExt(const std::string& format) noexcept
	{
		auto itr = _textureFormatExts.find(format);
		if (itr == _textureFormatExts.end())
		{
			return std::string(".") + format;
		}
		return itr->second;
	}

	const std::unordered_map<std::string, std::string> TexturePackerTextureAtlasImporter::_sheetFormatExts = {
		{ "rmlui", ".rcss"},
		{ "libRocket", ".rcss"},
	};

	std::string TexturePackerTextureAtlasImporter::getSheetFormatExt(const std::string& format) noexcept
	{
		auto itr = _sheetFormatExts.find(format);
		if (itr == _sheetFormatExts.end())
		{
			return std::string(".") + format;
		}
		return itr->second;
	}

	const std::string TexturePackerTextureAtlasImporter::_outputFormatOption = "outputFormat";
	const std::string TexturePackerTextureAtlasImporter::_textureFormatOption = "textureFormat";
	const std::string TexturePackerTextureAtlasImporter::_rmluiResolutionOption = "rmluiResolution";
	const std::string TexturePackerTextureAtlasImporter::_rmluiNameFormatOption = "rmluiNameFormat";
	const std::string TexturePackerTextureAtlasImporter::_rmluiSpriteNameFormatOption = "rmluiSpriteNameFormat";
	const std::string TexturePackerTextureAtlasImporter::_rmluiBoxNameFormatOption = "rmluiBoxNameFormat";

	bool TexturePackerTextureAtlasImporter::startImport(const Input& input, bool dry)
	{
		if (input.config.is_null())
		{
			return false;
		}
		_xmlDoc.load_file(input.path.c_str());

		std::string sheetFormat;
		if (input.config.contains(_outputFormatOption))
		{
			sheetFormat = input.config[_outputFormatOption].get<std::string>();
		}
		auto basePath = input.getRelativePath().parent_path();

		_sheetPath = input.getOutputPath();
		if(_sheetPath.empty())
		{
			auto sheetNode = _xmlDoc.select_node(
				"//map[@type='GFileNameMap']/struct[@type='DataFile']"
				"/key[text()='name']/following::filename");
			if (sheetNode)
			{
				std::string val = sheetNode.node().text().as_string();
				if (!val.empty())
				{
					_sheetPath = basePath / val;
				}
			}
		}
		if (_sheetPath.empty())
		{
			if (sheetFormat.empty())
			{
				auto sheetFormatNode = _xmlDoc.select_node(
					"//key[text()='dataFormat']/following::string");
				if (sheetFormatNode)
				{
					sheetFormat = sheetFormatNode.node().text().as_string();
				}
			}
			auto ext = getSheetFormatExt(sheetFormat);
			_sheetPath = basePath / (input.path.stem().string() + ext);
		}

		basePath = _sheetPath.parent_path();

		auto textureNode = _xmlDoc.select_node(
			"//key[text()='textureSubPath']/following::string");
		if (textureNode)
		{
			std::string val = textureNode.node().text().as_string();
			if (!val.empty())
			{
				_texturePath = basePath / val;
			}
		}
		if(_texturePath.empty())
		{
			auto textureFormatNode = _xmlDoc.select_node(
				"//key[text()='textureFormat']/following::enum[@type='SettingsBase::TextureFormat']");

			std::string ext = ".png";
			if (textureFormatNode)
			{
				ext = getTextureFormatExt(textureFormatNode.node().text().as_string());
			}

			_texturePath = basePath / (_sheetPath.stem().string() + ext);
		}

		auto tempSpritesheetPath = getTempPath();
		auto tempTexturePath = getTempPath();

		std::vector<Exec::Arg> args{
			_exePath, input.path,
			"--sheet", tempTexturePath,
			"--data", tempSpritesheetPath,
			"--verbose"
		};

		auto convertRmlui = sheetFormat == "rmlui";
		if (convertRmlui)
		{
			sheetFormat = "xml";
		}
		
		if (!sheetFormat.empty())
		{
			args.push_back("--format");
			args.push_back(sheetFormat);
		}

		if (input.config.contains(_textureFormatOption))
		{
			args.push_back("--texture-format");
			args.push_back(input.config[_textureFormatOption].get<std::string>());
		}

		auto r = Exec::run(args);

		if (r.returnCode != 0)
		{
			if (_log)
			{
				*_log << "TexturePacker output:" << std::endl;
				*_log << r.out;
				*_log << "TexturePacker error output:" << std::endl;
				*_log << r.err;
			}
			throw std::runtime_error("failed to run texture packer");
		}

		_sheetData = Data::fromFile(tempSpritesheetPath);
		_textureData = Data::fromFile(tempTexturePath);

		if (convertRmlui)
		{
			pugi::xml_document doc;
			auto result = doc.load_buffer_inplace(_sheetData.ptr(), _sheetData.size());
			if (result.status != pugi::status_ok)
			{
				throw std::runtime_error(result.description());
			}
			TextureAtlasDefinition atlasDef;
			if (!atlasDef.readTexturePacker(doc, basePath))
			{
				throw std::runtime_error("failed to read texture packer xml");
			}
			atlasDef.imagePath = std::filesystem::relative(_texturePath, basePath);
			_sheetData.clear();
			DataOutputStream out(_sheetData);
			auto rmluiConfig = readRmluiConfig(input.config);
			atlasDef.writeRmlui(out, rmluiConfig);
			_sheetData.resize(out.tellp());
		}

		return true;
	}

	TextureAtlasRmluiConfig TexturePackerTextureAtlasImporter::readRmluiConfig(const nlohmann::json& json) noexcept
	{
		TextureAtlasDefinition::RmluiConfig config;
		if (json.contains(_rmluiNameFormatOption))
		{
			config.nameFormat = json[_rmluiNameFormatOption];
		}
		if (json.contains(_rmluiResolutionOption))
		{
			config.resolution = json[_rmluiResolutionOption];
		}
		if (json.contains(_rmluiSpriteNameFormatOption))
		{
			config.spriteNameFormat = json[_rmluiSpriteNameFormatOption];
		}
		if (json.contains(_rmluiBoxNameFormatOption))
		{
			config.boxNameFormat = json[_rmluiBoxNameFormatOption];
		}
		return config;
	}

	void TexturePackerTextureAtlasImporter::endImport(const Input& input)
	{
		_xmlDoc.reset();
		_sheetPath.clear();
		_texturePath.clear();
		_sheetData.clear();
		_textureData.clear();
	}

	TexturePackerTextureAtlasImporter::Outputs TexturePackerTextureAtlasImporter::getOutputs(const Input& input)
	{
		return { _sheetPath, _texturePath };
	}

	TexturePackerTextureAtlasImporter::Dependencies TexturePackerTextureAtlasImporter::getDependencies(const Input& input)
	{
		Dependencies deps;

		auto basePath = input.path.parent_path();

		auto nodes = _xmlDoc.select_nodes(
			"//struct[@type='SpriteSheet']/key[text()='files']"
			"/following::array/filename");
		for (auto& node : nodes)
		{
			auto path = node.node().text().as_string();
			deps.insert(basePath / path);
		}

		return deps;
	}

	void TexturePackerTextureAtlasImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
	{
		if (outputIndex == 0)
		{
			out << _sheetData;
		}
		else
		{
			out << _textureData;
		}
		
	}
}