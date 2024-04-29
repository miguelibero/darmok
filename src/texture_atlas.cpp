#include "texture_atlas.hpp"
#include <darmok/data.hpp>
#include <darmok/mesh.hpp>
#include <darmok/anim.hpp>
#include <darmok/vertex.hpp>
#include <darmok/texture.hpp>
#include <darmok/material.hpp>

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

	TextureAtlasBounds TextureAtlas::getBounds(std::string_view prefix) const noexcept
	{
		TextureAtlasBounds bounds{};
		for (auto& elm : elements)
		{
			if (elm.name.starts_with(prefix))
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

	TextureAtlasMeshCreator::TextureAtlasMeshCreator(const bgfx::VertexLayout& layout, const TextureAtlas& atlas) noexcept
		: layout(layout)
		, atlas(atlas)
		, config{}
	{
	}

	std::shared_ptr<Mesh> TextureAtlasMeshCreator::createSprite(const TextureAtlasElement& elm) const noexcept
	{
		uint32_t vertexAmount = elm.getVertexAmount();
		VertexDataWriter writer(layout, vertexAmount * config.amount.x * config.amount.y);
		std::vector<VertexIndex> indices;
		indices.reserve(elm.indices.size() * config.amount.x * config.amount.y);

		glm::vec2 fatlasSize(atlas.size);

		auto baseOffset = config.offset + glm::vec3(elm.offset, 0) - glm::vec3(elm.pivot * glm::vec2(elm.originalSize), 0);
		auto amountStep = glm::vec2(elm.originalSize);
		auto amountOffsetMax = (glm::vec2(config.amount) - glm::vec2(1)) * amountStep * 0.5F;
		auto amountOffset = -glm::vec3(amountOffsetMax, 0.F);

		uint32_t vertexIndex = 0;
		for (; amountOffset.x <= amountOffsetMax.x; amountOffset.x += amountStep.x)
		{
			amountOffset.y = -amountOffsetMax.y;
			for (; amountOffset.y <= amountOffsetMax.y; amountOffset.y += amountStep.y)
			{
				auto offset = baseOffset + amountOffset;
				for(uint32_t i = 0; i < vertexAmount; i++)
				{
					auto& texPos = elm.positions[i];
					auto pos = (offset + glm::vec3(texPos.x, float(elm.originalSize.y) - texPos.y, 0)) * config.scale;
					writer.write(bgfx::Attrib::Position, vertexIndex + i, pos);
					auto texCoord = glm::vec2(elm.texCoords[i]) / fatlasSize;
					writer.write(bgfx::Attrib::TexCoord0, vertexIndex + i, texCoord);
				}
				for (auto& idx : elm.indices)
				{
					indices.push_back(vertexIndex + idx);
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

		auto vertexData = writer.finish();
		return std::make_shared<Mesh>(layout, DataView(vertexData), DataView(indices));
	}

	std::shared_ptr<Material> TextureAtlasMeshCreator::createMaterial(const Color& color) const noexcept
	{
		auto material = std::make_shared<Material>();
		material->setTexture(MaterialTextureType::Diffuse, atlas.texture);
		material->setColor(MaterialColorType::Diffuse, color);
		return material;
	}

	std::shared_ptr<Mesh> TextureAtlasMeshCreator::createSprite(std::string_view name) const noexcept
	{
		auto elm = atlas.getElement(name);
		if (!elm)
		{
			return nullptr;
		}
		auto mesh = createSprite(elm.value());
		mesh->setMaterial(createMaterial(config.color));
		return mesh;
	}

	std::vector<AnimationFrame> TextureAtlasMeshCreator::createAnimation(std::string_view namePrefix, float frameDuration) const noexcept
	{
		auto material = createMaterial(config.color);
		std::vector<AnimationFrame> frames;

		for (auto& elm : atlas.elements)
		{
			if (elm.name.starts_with(namePrefix))
			{
				auto mesh = createSprite(elm);
				if (mesh)
				{
					mesh->setMaterial(material);
					frames.push_back({ { mesh }, frameDuration });
				}
			}
		}
		return frames;
	}

    TexturePackerTextureAtlasLoader::TexturePackerTextureAtlasLoader(IDataLoader& dataLoader, ITextureLoader& textureLoader) noexcept
		: _dataLoader(dataLoader)
		, _textureLoader(textureLoader)
	{
	}

	namespace texture_packer_xml
	{
		static std::pair<int, size_t> readInt(std::string_view str, size_t i) noexcept
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

		static std::vector<TextureAtlasIndex> readTextureIndexList(std::string_view str) noexcept
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

		static std::pair<std::optional<glm::uvec2>, size_t> readTextureVec2(std::string_view str, size_t i) noexcept
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

		static std::vector<glm::uvec2> readTextureVec2List(std::string_view data) noexcept
		{
			std::vector<glm::uvec2> list;
			size_t pi = 0;
			while (pi != std::string::npos)
			{
				auto pp = readTextureVec2(data, pi);
				if (!pp.first.has_value())
				{
					break;
				}
				list.push_back(pp.first.value());
				pi = pp.second;
			}
			return list;
		}
	}

	TextureAtlasElement loadElement(pugi::xml_node& xml, const glm::uvec2& textureSize) noexcept
	{
		TextureAtlasElement elm{ xml.attribute("n").value() };

		elm.texturePosition = glm::uvec2(0);
		elm.size = textureSize;

		auto xmlX = xml.attribute("x");
		if (xmlX)
		{
			elm.texturePosition.x = xmlX.as_int();
		}
		auto xmlY = xml.attribute("y");
		if (xmlY)
		{
			elm.texturePosition.y = xmlY.as_int();
		}
		auto xmlW = xml.attribute("w");
		if (xmlW)
		{
			elm.size.x = xmlW.as_int();
		}
		auto xmlH = xml.attribute("h");
		if (xmlH)
		{
			elm.size.y = xmlH.as_int();
		}

		elm.offset = glm::uvec2(0);
		auto xmlOriginalX = xml.attribute("oX");
		if (xmlOriginalX)
		{
			elm.offset.x = xmlOriginalX.as_int();
		}
		auto xmlOriginalY = xml.attribute("oY");
		if (xmlOriginalY)
		{
			elm.offset.y = xmlOriginalY.as_int();
		}

		auto xmlVertices = xml.child("vertices");
		if (xmlVertices)
		{
			elm.positions = texture_packer_xml::readTextureVec2List(xmlVertices.text().get());
		}
		else
		{
			elm.positions = {
				glm::uvec2(elm.size.x, 0), elm.size,
				glm::uvec2(0, elm.size.y), glm::uvec2(0),
			};
		}

		auto xmlVerticesUV = xml.child("verticesUV");
		if (xmlVerticesUV)
		{
			elm.texCoords = texture_packer_xml::readTextureVec2List(xmlVerticesUV.text().get());
		}
		else
		{
			elm.texCoords.reserve(elm.positions.size());
			for (auto& p : elm.positions)
			{
				elm.texCoords.push_back(elm.texturePosition + p);
			}
		}

		auto xmlTriangles = xml.child("triangles");
		if (xmlTriangles)
		{
			elm.indices = texture_packer_xml::readTextureIndexList(xmlTriangles.text().get());
		}
		else
		{
			elm.indices = { 0, 1, 2, 2, 3, 0 };
		}
		
		elm.originalSize = elm.size;
		auto xmlOriginalW = xml.attribute("oW");
		if (xmlOriginalW)
		{
			elm.originalSize.x = xmlOriginalW.as_int();
		}
		auto xmlOriginalH = xml.attribute("oH");
		if (xmlOriginalH)
		{
			elm.originalSize.y = xmlOriginalH.as_int();
		}

		elm.pivot = { xml.attribute("pX").as_float(), xml.attribute("pY").as_float() };
		elm.rotated = std::string(xml.attribute("r").value()) == "y";

		return elm;
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
		auto atlasXml = doc.child("TextureAtlas");
		auto imagePath = atlasXml.attribute("imagePath").value();

		std::filesystem::path p(imagePath);
		if (!p.has_parent_path())
		{
			p = std::filesystem::path(name).parent_path() / p;
		}
		auto atlasFullName = p.string();
		auto texture = _textureLoader(atlasFullName, textureFlags);

		glm::uvec2 size{
			atlasXml.attribute("width").as_int(),
			atlasXml.attribute("height").as_int(),
		};

		static const char* spriteTag = "sprite";
		std::vector<TextureAtlasElement> elements;
		for (pugi::xml_node spriteXml = atlasXml.child(spriteTag); spriteXml; spriteXml = spriteXml.next_sibling(spriteTag))
		{
			elements.push_back(loadElement(spriteXml, size));
		}

		return std::make_shared<TextureAtlas>(texture, elements, size);
	}
}