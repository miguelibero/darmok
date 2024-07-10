#include <darmok/texture_atlas.hpp>
#include <darmok/data.hpp>
#include <darmok/mesh.hpp>
#include <darmok/anim.hpp>
#include <darmok/vertex.hpp>
#include <darmok/texture.hpp>
#include <darmok/string.hpp>
#include <darmok/shape.hpp>

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

	void TextureAtlasElement::read(const pugi::xml_node& xml, const glm::uvec2& textureSize) noexcept
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

	void TextureAtlasElement::write(pugi::xml_node& xml) const noexcept
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

	bool TextureAtlasData::read(const pugi::xml_document& doc, const std::filesystem::path& basePath)
	{
		if (doc.empty())
		{
			return false;
		}
		return read(doc.child("TextureAtlas"), basePath);
	}

	bool TextureAtlasData::read(const pugi::xml_node& atlasXml, const std::filesystem::path& basePath)
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
			elements.emplace_back().read(spriteXml, size);
		}

		return true;
	}

	void TextureAtlasData::write(pugi::xml_document& doc) const noexcept
	{
		pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
		decl.append_attribute("version") = "1.0";
		decl.append_attribute("encoding") = "utf-8";
		auto node = doc.append_child("TextureAtlas");
		write(node);
	}

	void TextureAtlasData::write(pugi::xml_node& xml) const noexcept
	{
		xml.append_attribute("imagePath") = imagePath.string().c_str();
		for (auto& elm : elements)
		{
			auto sprite = xml.append_child("sprite");
			elm.write(sprite);
		}
	}

    TexturePackerTextureAtlasLoader::TexturePackerTextureAtlasLoader(IDataLoader& dataLoader, ITextureLoader& textureLoader) noexcept
		: _dataLoader(dataLoader)
		, _textureLoader(textureLoader)
	{
	}	

	std::shared_ptr<TextureAtlas> TexturePackerTextureAtlasLoader::operator()(std::string_view name, uint64_t textureFlags)
	{
		auto data = _dataLoader(name);
		if (data.empty())
		{
			throw std::runtime_error("got empty data");
		}

		pugi::xml_document doc;
		auto result = doc.load_buffer_inplace(data.ptr(), data.size());
		if (result.status != pugi::status_ok)
		{
			throw std::runtime_error(result.description());
		}
		TextureAtlasData atlasData;
		if (!atlasData.read(doc, std::filesystem::path(name).parent_path()))
		{
			throw std::runtime_error("failed to read texture packer xml");
		}

		auto texture = _textureLoader(atlasData.imagePath.string(), textureFlags);
		return std::make_shared<TextureAtlas>(TextureAtlas{ texture, atlasData.elements });
	}
}