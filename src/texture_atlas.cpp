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
#include <darmok/glm_serialize.hpp>
#include <bx/platform.h>

#include <filesystem>
#include <charconv>
#include <pugixml.hpp>

namespace darmok
{
	namespace TextureAtlasDetail
	{
		std::pair<int, size_t> readInt(std::string_view str, size_t i) noexcept
		{
			if (str.empty() || i == std::string::npos || i >= str.size())
			{
				return { -1, std::string::npos };
			}
			auto pos = str.find(' ', i);
			if (pos == std::string::npos)
			{
				pos = str.size();
			}
			int val = -1;
			std::from_chars(str.data() + i, str.data() + pos, val);
			return { val, pos + 1 };
		}

		std::vector<int> readIndexList(std::string_view str) noexcept
		{
			std::vector<int> list;
			size_t pos = 0;
			while (pos != std::string::npos)
			{
				auto [val, nextPos] = readInt(str, pos);
				if (val < 0)
				{
					break;
				}
				list.push_back(val);
				pos = nextPos;
			}
			return list;
		}

		std::pair<std::optional<glm::uvec2>, size_t> readUvec2(std::string_view str, size_t pos) noexcept
		{
			auto [intVal, intPos] = readInt(str, pos);
			if (intVal < 0)
			{
				return { std::nullopt, std::string::npos };
			}
			glm::uvec2 val = { intVal, 0 };
			pos = intPos;
			if (pos == std::string::npos)
			{
				return { val, pos };
			}
			auto [intVal2, intPos2] = readInt(str, pos);
			if (intVal2 < 0)
			{
				return { std::nullopt, std::string::npos };
			}
			val.y = intVal2;
			pos = intPos2;
			return { val, pos };
		}

		std::vector<glm::uvec2> readUvec2List(std::string_view data) noexcept
		{
			std::vector<glm::uvec2> list;
			size_t i = 0;
			while (i != std::string::npos)
			{
				auto [val, nextPos] = readUvec2(data, i);
				if (!val.has_value())
				{
					break;
				}
				list.push_back(val.value());
				i = nextPos;
			}
			return list;
		}

		std::string writeUvec2List(const std::vector<glm::uvec2>& list) noexcept
		{
			return StringUtils::join(" ", list.begin(), list.end(), [](auto& v) {
				return std::to_string(v.x) + " " + std::to_string(v.y);
			});
		}

		std::string writeIndexList(const std::vector<int>& list) noexcept
		{
			return StringUtils::join(" ", list.begin(), list.end(), [](auto& v) {
				return std::to_string(v);
			});
		}
	}

	namespace TextureAtlasUtils
	{
		Bounds getBounds(const Element& elm) noexcept
		{
			return {
				convert<glm::uvec2>(elm.original_size()),
				convert<glm::vec2>(elm.offset())
			};
		}

		size_t getVertexAmount(const Element& elm) noexcept
		{
			return std::min(elm.positions_size(), elm.texture_coords_size());
		}

		Element createElement(const Bounds& bounds) noexcept
		{
			static const std::vector<glm::uvec2> positions = { { 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 } };
			static const std::vector<VertexIndex> indices = { 0, 1, 2, 2, 3, 0 };
			Element elm;
			
			for (auto& pos : positions)
			{
				*elm.add_positions() = convert<protobuf::Uvec2>(pos * bounds.size);
				auto texCoord = pos;
				texCoord.y = texCoord.y ? 0 : 1;
				texCoord = bounds.offset + texCoord * bounds.size;
				*elm.add_texture_coords() = convert<protobuf::Uvec2>(texCoord);
			}
			for (auto& idx : indices)
			{
				elm.add_indices(idx);
			}
			*elm.mutable_texture_position() = convert<protobuf::Uvec2>(bounds.offset);
			auto protoSize = convert<protobuf::Uvec2>(bounds.size);
			*elm.mutable_size() = protoSize;
			*elm.mutable_original_size() = protoSize;

			return elm;
		}

		expected < std::unique_ptr<Mesh>, std::string> createSprite(const Element& elm, const bgfx::VertexLayout& layout, const glm::uvec2& textureSize, const MeshConfig& config) noexcept
		{
			auto vertexAmount = static_cast<uint32_t>(getVertexAmount(elm));
			VertexDataWriter writer(layout, vertexAmount * config.amount.x * config.amount.y);
			std::vector<VertexIndex> totalIndices;
			totalIndices.reserve(elm.indices_size() * config.amount.x * config.amount.y);

			const glm::vec2 fatlasSize{ textureSize };

			auto pivot = convert<glm::vec2>(elm.pivot());
			auto originalSize = convert<glm::uvec2>(elm.original_size());
			auto baseOffset = config.offset - glm::vec3{ pivot * glm::vec2{originalSize}, 0 };
			// don't think this is needed since it's already factored into the position values
			// baseOffset += glm::vec3(offset, 0);
			auto amountStep = glm::vec2{ originalSize };
			auto amountOffsetMax = (glm::vec2{ config.amount } - glm::vec2{ 1 }) * amountStep * 0.5F;
			auto amountOffset = -glm::vec3{ amountOffsetMax, 0.F };

			uint32_t vertexIndex = 0;
			for (; amountOffset.x <= amountOffsetMax.x; amountOffset.x += amountStep.x)
			{
				amountOffset.y = -amountOffsetMax.y;
				for (; amountOffset.y <= amountOffsetMax.y; amountOffset.y += amountStep.y)
				{
					auto elmOffset = baseOffset + amountOffset;
					for (uint32_t i = 0; i < vertexAmount; i++)
					{
						auto texPos = convert<glm::uvec2>(elm.positions()[i]);
						auto texCoord = convert<glm::uvec2>(elm.texture_coords()[i]);
						auto pos = (elmOffset + glm::vec3(texPos.x, float(originalSize.y) - texPos.y, 0)) * config.scale;
						writer.write(bgfx::Attrib::Position, vertexIndex + i, pos);
						auto ftexCoord = glm::vec2(texCoord) / fatlasSize;
						writer.write(bgfx::Attrib::TexCoord0, vertexIndex + i, ftexCoord);
					}
					for (const auto& idx : elm.indices())
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

			const Data vertexData = writer.finish();
			Mesh::Config meshConfig{ .type = config.type };
			auto meshResult = Mesh::load(layout, DataView{ vertexData }, DataView{ totalIndices }, meshConfig);
			if (!meshResult)
			{
				return unexpected{ std::move(meshResult).error() };
			}
			return std::make_unique<Mesh>(std::move(meshResult).value());
		}

		bool isRect(const Element& elm) noexcept
		{
			if (elm.positions_size() != 4)
			{
				return false;
			}
			if (convert<glm::uvec2>(elm.positions()[0]) != glm::uvec2{ elm.size().x(), 0 })
			{
				return false;
			}
			if (convert<glm::uvec2>(elm.positions()[1]) != convert<glm::uvec2>(elm.size()))
			{
				return false;
			}
			if (convert<glm::uvec2>(elm.positions()[2]) != glm::uvec2{ 0, elm.size().y() })
			{
				return false;
			}
			if (convert<glm::uvec2>(elm.positions()[3]) != glm::uvec2{ 0 })
			{
				return false;
			}
			return true;
		}

		void readTexturePacker(Element& elm, const pugi::xml_node& xml, const glm::uvec2& textureSize) noexcept
		{
			elm.set_name(xml.attribute("n").value());

			auto& texPos = *elm.mutable_texture_position();
			auto& size = *elm.mutable_size();
			auto& offset = *elm.mutable_offset();
			auto& originalSize = *elm.mutable_original_size();
			auto& pivot = *elm.mutable_pivot();

			auto xmlX = xml.attribute("x");
			texPos.set_x(xmlX ? xmlX.as_int() : 0);
			auto xmlY = xml.attribute("y");
			texPos.set_y(xmlY ? xmlY.as_int() : 0);
			auto xmlW = xml.attribute("w");
			size.set_x(xmlW ? xmlW.as_int() : textureSize.x);
			auto xmlH = xml.attribute("h");
			size.set_y(xmlH ? xmlH.as_int() : textureSize.y);
			auto xmlOriginalX = xml.attribute("oX");
			offset.set_x(xmlOriginalX ? xmlOriginalX.as_float() : 0);
			auto xmlOriginalY = xml.attribute("oY");
			offset.set_y(xmlOriginalY ? xmlOriginalY.as_float() : 0);
			auto xmlOriginalW = xml.attribute("oW");
			originalSize.set_x(xmlOriginalW ? xmlOriginalW.as_int() : size.x());
			auto xmlOriginalH = xml.attribute("oH");
			originalSize.set_y(xmlOriginalH ? xmlOriginalH.as_int() : size.y());
			pivot.set_x(xml.attribute("pX").as_float());
			pivot.set_y(xml.attribute("pY").as_float());
			elm.set_rotated(std::string(xml.attribute("r").value()) == "y");

			auto xmlVertices = xml.child("vertices");
			if (xmlVertices)
			{
				for (const auto& pos : TextureAtlasDetail::readUvec2List(xmlVertices.text().get()))
				{
					*elm.add_positions() = convert<protobuf::Uvec2>(pos);
				}
			}
			else
			{
				*elm.add_positions() = convert<protobuf::Uvec2>(glm::uvec2{ size.x(), 0 });
				*elm.add_positions() = size;
				*elm.add_positions() = convert<protobuf::Uvec2>(glm::uvec2{ 0, size.y() });
				*elm.add_positions() = convert<protobuf::Uvec2>(glm::uvec2{ 0 });
			}

			auto xmlVerticesUV = xml.child("verticesUV");
			if (xmlVerticesUV)
			{
				for (const auto& texCoord : TextureAtlasDetail::readUvec2List(xmlVerticesUV.text().get()))
				{
					*elm.add_texture_coords() = convert<protobuf::Uvec2>(texCoord);
				}
			}
			else
			{
				elm.mutable_texture_coords()->Reserve(elm.positions_size());
				for (auto& pos : elm.positions())
				{
					auto& texCoord = *elm.add_texture_coords();
					texCoord.set_x(texPos.x() + pos.x());
					texCoord.set_y(texPos.y() + pos.y());
				}
			}

			std::vector<int> indices = { 0, 1, 2, 2, 3, 0 };
			auto xmlTriangles = xml.child("triangles");
			if (xmlTriangles)
			{
				indices = TextureAtlasDetail::readIndexList(xmlTriangles.text().get());
			}
			for (auto idx : indices)
			{
				elm.mutable_indices()->Add(idx);
			}
		}

		void writeTexturePacker(const Element& elm, pugi::xml_node& xml) noexcept
		{
			xml.append_attribute("n") = elm.name();
			xml.append_attribute("x") = elm.texture_position().x();
			xml.append_attribute("y") = elm.texture_position().y();
			xml.append_attribute("w") = elm.size().x();
			xml.append_attribute("h") = elm.size().y();
			if (elm.offset().x() != 0)
			{
				xml.append_attribute("oX") = elm.offset().x();
			}
			if (elm.offset().y() != 0)
			{
				xml.append_attribute("oY") = elm.offset().y();
			}
			if (elm.original_size().x() != 0)
			{
				xml.append_attribute("oW") = elm.original_size().x();
			}
			if (elm.original_size().y() != 0)
			{
				xml.append_attribute("oH") = elm.original_size().y();
			}
			if (elm.pivot().x() != 0)
			{
				xml.append_attribute("pX") = elm.pivot().x();
			}
			if (elm.pivot().y() != 0)
			{
				xml.append_attribute("pY") = elm.pivot().y();
			}
			if (elm.rotated())
			{
				xml.append_attribute("r") = "y";
			}
			if (!isRect(elm))
			{
				auto convertUvec2 = convert<glm::uvec2, protobuf::Uvec2>;

				if (elm.positions_size() > 0)
				{
					std::vector<glm::uvec2> positions(elm.positions_size());
					std::transform(elm.positions().begin(), elm.positions().end(), positions.begin(), convertUvec2);
					xml.append_child("vertices").set_value(TextureAtlasDetail::writeUvec2List(positions));
				}
				if (elm.texture_coords_size() > 0)
				{
					std::vector<glm::uvec2> texCoords(elm.texture_coords_size());
					std::transform(elm.texture_coords().begin(), elm.texture_coords().end(), texCoords.begin(), convertUvec2);
					xml.append_child("verticesUV").set_value(TextureAtlasDetail::writeUvec2List(texCoords));
				}
				if (elm.indices_size() > 0)
				{
					std::vector<int> indices(elm.indices().begin(), elm.indices().end());
					xml.append_child("triangles").set_value(TextureAtlasDetail::writeIndexList(indices));
				}
			}
		}

		expected<void, std::string> readTexturePacker(Atlas& atlas, const pugi::xml_document& doc, const std::filesystem::path& basePath) noexcept
		{
			if (doc.empty())
			{
				return unexpected<std::string>("empty xml document");
			}
			return readTexturePacker(atlas, doc.child("TextureAtlas"), basePath);
		}

		expected<void, std::string> readTexturePacker(Atlas& atlas, const pugi::xml_node& node, const std::filesystem::path& basePath) noexcept
		{
			if (node.empty())
			{
				return unexpected<std::string>("empty xml node");
			}
			auto path = basePath / std::filesystem::path(node.attribute("imagePath").value());
			atlas.set_texture_path(path.string());

			glm::uvec2 size {
				node.attribute("width").as_int(),
				node.attribute("height").as_int(),
			};

			static const char* spriteTag = "sprite";
			for (pugi::xml_node spriteXml = node.child(spriteTag); spriteXml; spriteXml = spriteXml.next_sibling(spriteTag))
			{
				readTexturePacker(*atlas.add_elements(), spriteXml, size);
			}

			return {};
		}

		expected<void, std::string> writeTexturePacker(const Atlas& atlas, pugi::xml_document& doc, const std::filesystem::path& basePath, const std::optional<ImageLoadContext>& imgLoad) noexcept
		{
			pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
			decl.append_attribute("version") = "1.0";
			decl.append_attribute("encoding") = "utf-8";
			auto node = doc.append_child("TextureAtlas");
			return writeTexturePacker(atlas, node, basePath, imgLoad);
		}

		expected<void, std::string> writeTexturePacker(const Atlas& atlas, pugi::xml_node& node, const std::filesystem::path& basePath, const std::optional<ImageLoadContext>& imgLoad) noexcept
		{
			auto imagePath = basePath / atlas.texture_path();
			node.append_attribute("imagePath") = imagePath.string();
			if (imgLoad)
			{
				auto texResult = imgLoad->texLoader(imagePath);
				if (!texResult)
				{
					return unexpected{ texResult.error() };
				}
				auto imgResult = TextureDefinitionWrapper{ *texResult.value() }.writeImage(imgLoad->alloc, imagePath);
				if (!imgResult)
				{
					return unexpected{ imgResult.error() };
				}
			}
			for (const auto& elm : atlas.elements())
			{
				auto sprite = node.append_child("sprite");
				writeTexturePacker(elm, sprite);
			}
			return {};
		}

		expected<void, std::string> writeRmlui(const Atlas& atlas, std::ostream& out, const RmluiConfig& config) noexcept
		{
			auto imagePath = std::filesystem::path(atlas.texture_path());
			auto name = imagePath.stem().string();
			if (!config.nameFormat.empty())
			{
				const std::string origName = name;
				name = config.nameFormat;
				StringUtils::replace(name, "*", origName);
			}
			out << "@spritesheet " << name << "\n";
			out << "{\n";
			out << "    src: " << imagePath.string() << ";\n";
			out << "    resolution: " << config.resolution << "x;\n";

			for (const auto& elm : atlas.elements())
			{
				auto& pos = elm.texture_position();
				auto& size = elm.size();
				const glm::uvec4 val(pos.x(), pos.y(), size.x(), size.y());
				auto name = std::filesystem::path{ elm.name() }.stem().string();
				if (!config.spriteNameFormat.empty())
				{
					const std::string origName = name;
					name = config.spriteNameFormat;
					StringUtils::replace(name, "*", origName);
				}
				out << "    " << name << ": " << val.x << "px " << val.y << "px " << val.z << "px " << val.w << "px;\n";
			}
			out << "}\n";

			if (!config.boxNameFormat.empty())
			{
				for (const auto& elm : atlas.elements())
				{
					out << "\n";
					std::string name = config.boxNameFormat;
					auto origName = std::filesystem::path{ elm.name() }.stem().string();
					StringUtils::replace(name, "*", origName);

					out << "." << name << " {\n";
					out << "  width: " << elm.original_size().x() << "px;\n";
					out << "  height: " << elm.original_size().y() << "px;\n";
					out << "  padding-left: " << elm.offset().x() << "px;\n";
					out << "  padding-bottom: " << elm.offset().y() << "px;\n";
					out << "}\n";
				}
			}
			if (out.fail())
			{
				return unexpected<std::string>{ "failed to write rmlui file" };
			}
			return {};
		}
	}

	TextureAtlasBounds TextureAtlas::getBounds(std::string_view prefix) const noexcept
	{
		TextureAtlasBounds bounds{};
		for (const auto& elm : elements)
		{
			if (StringUtils::startsWith(elm.name(), prefix))
			{
				// TODO: algorithm to combine bounds
				bounds = TextureAtlasUtils::getBounds(elm);
			}
		}
		return bounds;
	}

	OptionalRef<TextureAtlas::Element> TextureAtlas::getElement(std::string_view name) noexcept
	{
		for (auto& elm : elements)
		{
			if (elm.name() == name)
			{
				return elm;
			}
		}
		return nullptr;
	}

	OptionalRef<const TextureAtlas::Element> TextureAtlas::getElement(std::string_view name) const noexcept
	{
		for (const auto& elm : elements)
		{
			if (elm.name() == name)
			{
				return elm;
			}
		}
		return nullptr;
	}

	expected<std::unique_ptr<Mesh>, std::string> TextureAtlas::createSprite(std::string_view name, const bgfx::VertexLayout& layout, const MeshConfig& config) const noexcept
	{
		auto elm = getElement(name);
		if (!elm)
		{
			return unexpected<std::string>{ "missing element \"" + std::string{ name } + "\"" };
		}
		const auto& size = texture->getSize();
		return TextureAtlasUtils::createSprite(*elm, layout, size, config);
	}

	std::vector<AnimationFrame> TextureAtlas::createAnimation(const bgfx::VertexLayout& layout, std::string_view namePrefix, float frameDuration, const MeshConfig& config) const noexcept
	{
		std::vector<AnimationFrame> frames;
		const auto& size = texture->getSize();

		for (const auto& elm : elements)
		{
			if (StringUtils::startsWith(elm.name(), namePrefix))
			{
				if (auto meshResult = TextureAtlasUtils::createSprite(elm, layout, size, config))
				{
					frames.emplace_back(std::move(meshResult).value(), frameDuration);
				}
			}
		}
		return frames;
	}

	TexturePackerDefinitionLoader::TexturePackerDefinitionLoader(IDataLoader& dataLoader, ITextureDefinitionLoader& texLoader) noexcept
		: _dataLoader{ dataLoader }
		, _texLoader{ texLoader }
	{
	}

	TexturePackerDefinitionLoader::Result TexturePackerDefinitionLoader::operator()(std::filesystem::path path) noexcept
	{
		auto dataResult = _dataLoader(path);
		if (!dataResult)
		{
			return unexpected{ dataResult.error() };
		}

		auto data = std::move(dataResult.value());

		pugi::xml_document doc;
		auto xmlResult = doc.load_buffer_inplace(data.ptr(), data.size());
		if (xmlResult.status != pugi::status_ok)
		{
			return unexpected<std::string>{ xmlResult.description() };
		}
		auto atlasDef = std::make_shared<TextureAtlas::Definition>();
		auto readResult = TextureAtlasUtils::readTexturePacker(*atlasDef, doc, path.parent_path());
		if (!readResult)
		{
			return unexpected<std::string>{"failed to read texture packer xml: " + readResult.error()};
		}
		atlasDef->set_texture_path(atlasDef->texture_path());
		return atlasDef;
	}

	TextureAtlasLoader::TextureAtlasLoader(ITextureAtlasDefinitionLoader& defLoader, ITextureLoader& texLoader) noexcept
		: FromDefinitionLoader(defLoader)
		, _texLoader{ texLoader }
	{
	}

	TextureAtlasLoader::Result TextureAtlasLoader::create(std::shared_ptr<TextureAtlas::Definition> def) noexcept
	{
		auto texResult = _texLoader(def->texture_path());
		if (!texResult)
		{
			return unexpected<std::string>{ texResult.error() };
		}
		auto atlas = std::make_shared<TextureAtlas>();
		atlas->elements.insert(atlas->elements.end(), def->elements().begin(), def->elements().end());
		atlas->texture = texResult.value();
		return atlas;
	}

	TexturePackerAtlasFileImporter::TexturePackerAtlasFileImporter(std::filesystem::path exePath) noexcept
		: _exePath{ std::move(exePath) }
	{
		if (_exePath.empty())
		{
			_exePath = "TexturePacker";
		}
#if BX_PLATFORM_WINDOWS
		_exePath += ".exe";
#endif
	}

	void TexturePackerAtlasFileImporter::setLogOutput(OptionalRef<std::ostream> log) noexcept
	{
		_log = log;
	}

	const std::string& TexturePackerAtlasFileImporter::getName() const noexcept
	{
		static const std::string name("texture_packer");
		return name;
	}

	const std::unordered_map<std::string, std::string> TexturePackerAtlasFileImporter::_textureFormatExts = {
		{ "png8", ".png"},
		{ "pvr3", ".pvr"},
		{ "pvr3gz", ".pvr.gz"},
		{ "pvr3ccz", ".pvr.ccz"},
		{ "pvr3ccz", ".pvr.ccz"},
	};

	std::string TexturePackerAtlasFileImporter::getTextureFormatExt(const std::string& format) noexcept
	{
		auto itr = _textureFormatExts.find(format);
		if (itr == _textureFormatExts.end())
		{
			return std::string(".") + format;
		}
		return itr->second;
	}

	const std::unordered_map<std::string, std::string> TexturePackerAtlasFileImporter::_sheetFormatExts = {
		{ "rmlui", ".rcss"},
		{ "libRocket", ".rcss"},
	};

	std::string TexturePackerAtlasFileImporter::getSheetFormatExt(const std::string& format) noexcept
	{
		auto itr = _sheetFormatExts.find(format);
		if (itr == _sheetFormatExts.end())
		{
			return std::string(".") + format;
		}
		return itr->second;
	}

	const std::string TexturePackerAtlasFileImporter::_outputFormatOption = "outputFormat";
	const std::string TexturePackerAtlasFileImporter::_textureFormatOption = "textureFormat";
	const std::string TexturePackerAtlasFileImporter::_rmluiResolutionOption = "rmluiResolution";
	const std::string TexturePackerAtlasFileImporter::_rmluiNameFormatOption = "rmluiNameFormat";
	const std::string TexturePackerAtlasFileImporter::_rmluiSpriteNameFormatOption = "rmluiSpriteNameFormat";
	const std::string TexturePackerAtlasFileImporter::_rmluiBoxNameFormatOption = "rmluiBoxNameFormat";

	expected<TexturePackerAtlasFileImporter::Effect, std::string> TexturePackerAtlasFileImporter::prepare(const Input& input) noexcept
	{
		Effect effect;
		if (input.config.is_null())
		{
			return effect;
		}
		_xmlDoc.load_file(input.path.c_str());

		_sheetFormat = {};
		if (input.config.contains(_outputFormatOption))
		{
			_sheetFormat = input.config[_outputFormatOption].get<std::string>();
		}
		auto basePath = input.getRelativePath().parent_path();

		auto sheetPath = input.getOutputPath();
		if (sheetPath.empty())
		{
			auto sheetNode = _xmlDoc.select_node(
				"//map[@type='GFileNameMap']/struct[@type='DataFile']"
				"/key[text()='name']/following::filename");
			if (sheetNode)
			{
				const std::string val = sheetNode.node().text().as_string();
				if (!val.empty())
				{
					sheetPath = basePath / val;
				}
			}
		}
		if (sheetPath.empty())
		{
			if (_sheetFormat.empty())
			{
				auto sheetFormatNode = _xmlDoc.select_node(
					"//key[text()='dataFormat']/following::string");
				if (sheetFormatNode)
				{
					_sheetFormat = sheetFormatNode.node().text().as_string();
				}
			}
			auto ext = getSheetFormatExt(_sheetFormat);
			sheetPath = basePath / (input.path.stem().string() + ext);
		}

		basePath = sheetPath.parent_path();

		auto textureNode = _xmlDoc.select_node(
			"//key[text()='textureSubPath']/following::string");
		std::filesystem::path texturePath;
		if (textureNode)
		{
			const std::string val = textureNode.node().text().as_string();
			if (!val.empty())
			{
				texturePath = basePath / val;
			}
		}
		if (texturePath.empty())
		{
			auto textureFormatNode = _xmlDoc.select_node(
				"//key[text()='textureFormat']/following::enum[@type='SettingsBase::TextureFormat']");

			std::string ext = ".png";
			if (textureFormatNode)
			{
				ext = getTextureFormatExt(textureFormatNode.node().text().as_string());
			}

			texturePath = basePath / (sheetPath.stem().string() + ext);
		}

		effect.outputs.emplace_back(sheetPath, false);
		effect.outputs.emplace_back(texturePath, true);
		_texturePath = texturePath;

		auto nodes = _xmlDoc.select_nodes(
			"//struct[@type='SpriteSheet']/key[text()='files']"
			"/following::array/filename");
		for (const auto& node : nodes)
		{
			const auto* path = node.node().text().as_string();
			effect.dependencies.insert(basePath / path);
		}

		return effect;
	}

	expected<void, std::string> TexturePackerAtlasFileImporter::operator()(const Input& input, Config& config) noexcept
	{
		auto tempSheetPath = getTempPath();
		auto tempTexturePath = getTempPath();

		std::vector<Exec::Arg> args{
			_exePath, input.path,
			"--sheet", tempTexturePath,
			"--data", tempSheetPath,
			"--verbose"
		};

		auto convertRmlui = _sheetFormat == "rmlui";
		if (convertRmlui)
		{
			_sheetFormat = "xml";
		}

		if (!_sheetFormat.empty())
		{
			args.emplace_back("--format");
			args.emplace_back(_sheetFormat);
		}

		if (input.config.contains(_textureFormatOption))
		{
			args.emplace_back("--texture-format");
			args.emplace_back(input.config[_textureFormatOption].get<std::string>());
		}

		auto result = Exec::run(args);

		if (result.returnCode != 0)
		{
			if (_log)
			{
				*_log << "TexturePacker output:\n";
				*_log << result.out;
				*_log << "TexturePacker error output:\n";
				*_log << result.err;
			}
			return unexpected{ "failed to run texture packer" };
		}
		auto dataResult = Data::fromFile(tempSheetPath);
		if (!dataResult)
		{
			return unexpected{ dataResult.error() };
		}
		auto sheetData = std::move(dataResult).value();
		dataResult = Data::fromFile(tempTexturePath);
		if (!dataResult)
		{
			return unexpected{ dataResult.error() };
		}
		auto textureData = std::move(dataResult).value();

		if (convertRmlui)
		{
			pugi::xml_document doc;
			auto xmlResult = doc.load_buffer_inplace(sheetData.ptr(), sheetData.size());
			if (xmlResult.status != pugi::status_ok)
			{
				return unexpected{ xmlResult.description() };
			}
			TextureAtlas::Definition atlasDef;
			auto basePath = input.getRelativePath().parent_path();
			auto texPackResult = TextureAtlasUtils::readTexturePacker(atlasDef, doc, basePath);
			if (!texPackResult)
			{
				return unexpected{ "failed to read texture packer xml: " + texPackResult.error() };
			}
			atlasDef.set_texture_path(std::filesystem::relative(_texturePath, basePath).string());
			sheetData.clear();
			DataOutputStream out{ sheetData };
			auto rmluiConfig = readRmluiConfig(input.config);

			auto rmlResult = TextureAtlasUtils::writeRmlui(atlasDef, out, rmluiConfig);
			if (!rmlResult)
			{
				return unexpected{ "failed to write rmlui: " + rmlResult.error() };
			}
			sheetData.resize(out.tellp());
		}

		if(auto& out = config.outputStreams[0])
		{
			*out << sheetData;
		}
		if (auto& out = config.outputStreams[1])
		{
			*out << textureData;
		}
		return {};
	}

	TextureAtlasRmluiConfig TexturePackerAtlasFileImporter::readRmluiConfig(const nlohmann::json& json) noexcept
	{
		TextureAtlasRmluiConfig config;
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

}