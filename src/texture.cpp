#include "texture.hpp"
#include <darmok/data.hpp>
#include <darmok/mesh.hpp>
#include <darmok/anim.hpp>
#include <darmok/vertex.hpp>
#include <filesystem>
#include <charconv>
#include <pugixml.hpp>


namespace darmok
{
	Texture::Texture(std::shared_ptr<Image> img, const bgfx::TextureHandle& handle, TextureType type) noexcept
		: _img(img)
		, _handle(handle)
		, _type(type)
	{
	}

	Texture::~Texture() noexcept
	{
		if (isValid(_handle))
		{
			bgfx::destroy(_handle);
		}
	}

	std::shared_ptr<Texture> Texture::create(std::shared_ptr<Image> img, std::string_view name, uint64_t flags) noexcept
	{
		if (img == nullptr || img->empty())
		{
			return nullptr;
		}

		const auto mem = img->makeRef();
		auto format = bgfx::TextureFormat::Enum(img->getFormat());
		auto hasMips = 1 < img->getMipCount();
		auto s = img->getSize();
		auto w = uint16_t(s.x);
		auto h = uint16_t(s.y);
		auto layers = img->getLayerCount();

		bgfx::TextureHandle handle{ bgfx::kInvalidHandle };
		TextureType type = TextureType::Unknown;

		if (img->isCubeMap())
		{
			handle = bgfx::createTextureCube(w, hasMips, layers, format, flags, mem);
			type = TextureType::CubeMap;
		}
		else if (1 < img->getDepth())
		{
			handle = bgfx::createTexture3D(w, h, uint16_t(img->getDepth()), hasMips, format, flags, mem);
			type = TextureType::Texture3D;
		}
		else if (bgfx::isTextureValid(0, false, layers, format, flags))
		{
			handle = bgfx::createTexture2D(w, h, hasMips, layers, format, flags, mem);
			type = TextureType::Texture2D;
		}

		if (!bgfx::isValid(handle))
		{
			return nullptr;
		}
		if (!name.empty())
		{
			bgfx::setName(handle, name.data(), name.size());
		}

		return std::make_shared<Texture>(img, handle, type);
	}

	const bgfx::TextureHandle& Texture::getHandle() const noexcept
	{
		return _handle;
	}

	std::shared_ptr<Image> Texture::getImage() const noexcept
	{
		return _img;
	}

	void Texture::releaseImage() noexcept
	{
		_img = nullptr;
		// TODO: is there a way to check if the image was uploaded?
	}

	TextureType Texture::getType() const noexcept
	{
		return _type;
	}

    TextureBounds TextureAtlasElement::getBounds() const noexcept
	{
		return {
			originalSize,
			offset
		};
	}

	TextureBounds TextureAtlas::getBounds(std::string_view prefix) const noexcept
	{
		TextureBounds bounds{};
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

	std::shared_ptr<Mesh> TextureAtlasElement::createSprite(const bgfx::VertexLayout& layout, const glm::uvec2& atlasSize,
		const SpriteCreationConfig& cfg) const noexcept
	{		
		auto elmSize = std::min(positions.size(), texCoords.size());
		VertexDataWriter writer(layout, elmSize);

		glm::vec2 fatlasSize(atlasSize);
		auto foffset = cfg.offset + glm::vec2(offset) - (pivot * glm::vec2(originalSize));

		uint32_t i = 0;
		for (auto& pos : positions)
		{
			auto v = foffset + glm::vec2(pos.x, float(originalSize.y) - pos.y);
			v *= cfg.scale;
			writer.write(bgfx::Attrib::Position, i++, v);
		}
		i = 0;
		for (auto& texCoord : texCoords)
		{
			auto v = cfg.textureScale * glm::vec2(texCoord) / fatlasSize;
			writer.write(bgfx::Attrib::TexCoord0, i++, v);
		}

		if (layout.has(bgfx::Attrib::Normal))
		{
			static const glm::vec3 norm(0, 0, -1);
			writer.write(bgfx::Attrib::Normal, norm);
		}

		auto vertexData = writer.finish();
		return std::make_shared<Mesh>(layout, std::move(vertexData), Data::copy(indices));
	}

	std::shared_ptr<Material> TextureAtlas::createMaterial(const Color& color) const noexcept
	{
		auto material = std::make_shared<Material>();
		material->setTexture(MaterialTextureType::Diffuse, texture);
		material->setColor(MaterialColorType::Diffuse, color);
		return material;
	}

	std::shared_ptr<Mesh> TextureAtlas::createSprite(const bgfx::VertexLayout& layout, const TextureAtlasElement& element, const SpriteCreationConfig& cfg) const noexcept
	{
		auto mesh = element.createSprite(layout, size, { cfg.scale, cfg.textureScale });
		mesh->setMaterial(createMaterial(cfg.color));
		return mesh;
	}

	std::vector<AnimationFrame> TextureAtlas::createSpriteAnimation(const bgfx::VertexLayout& layout, std::string_view namePrefix, float frameDuration, const SpriteCreationConfig& cfg) const noexcept
	{
		auto material = createMaterial(cfg.color);
		std::vector<AnimationFrame> frames;

		for (auto& elm : elements)
		{
			if (elm.name.starts_with(namePrefix))
			{
				auto mesh = elm.createSprite(layout, size, cfg);
				if (mesh)
				{
					mesh->setMaterial(material);
					frames.push_back({ { mesh }, frameDuration });
				}
			}
		}
		return frames;
	}

	ImageTextureLoader::ImageTextureLoader(IImageLoader& imgLoader) noexcept
		: _imgLoader(imgLoader)
	{
	}

	std::shared_ptr<Texture> ImageTextureLoader::operator()(std::string_view name, uint64_t flags) noexcept
	{
		auto img = _imgLoader(name);
		return Texture::create(img, name, flags);
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
			auto base = elm.offset + elm.texturePosition;
			elm.positions = {
				base + glm::uvec2(elm.size.x, 0), base + elm.size,
				base + glm::uvec2(0, elm.size.y), base,
			};
		}

		auto xmlVerticesUV = xml.child("verticesUV");
		if (xmlVerticesUV)
		{
			elm.texCoords = texture_packer_xml::readTextureVec2List(xmlVerticesUV.text().get());
		}
		else
		{
			elm.texCoords = elm.positions;
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

	std::shared_ptr<TextureAtlas> TexturePackerTextureAtlasLoader::operator()(std::string_view name, uint64_t flags)
	{
		auto data = _dataLoader(name);
		if (data == nullptr || data->empty())
		{
			throw std::runtime_error("got empty data");
		}

		pugi::xml_document doc;
		auto result = doc.load_buffer(data->ptr(), data->size());
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
		auto texture = _textureLoader(atlasFullName, flags);

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

	ColorTextureLoader::ColorTextureLoader(bx::AllocatorI* alloc, const glm::uvec2& size) noexcept
		: _alloc(alloc)
		, _size(size)
	{
	}

	std::shared_ptr<Texture> ColorTextureLoader::operator()(const Color& color) noexcept
	{
		auto itr = _cache.find(color);
		if (itr != _cache.end())
		{
			return itr->second;
		}
		auto tex = Texture::create(Image::create(_alloc, color, _size));
		_cache.emplace(color, tex);
		return tex;
	}

}