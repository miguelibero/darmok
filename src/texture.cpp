#include "texture.hpp"
#include <darmok/data.hpp>
#include <filesystem>
#include <charconv>
#include <pugixml.hpp>


namespace darmok
{
	Texture::Texture(std::shared_ptr<Image> img, const bgfx::TextureHandle& handle, TextureType type)
		: _img(img)
		, _handle(handle)
		, _type(type)
	{
	}

	Texture::~Texture()
	{
		if (isValid(_handle))
		{
			bgfx::destroy(_handle);
		}
	}

	std::shared_ptr<Texture> Texture::create(std::shared_ptr<Image> img, std::string_view name, uint64_t flags)
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

	const bgfx::TextureHandle& Texture::getHandle() const
	{
		return _handle;
	}

	std::shared_ptr<Image> Texture::getImage() const
	{
		return _img;
	}

	void Texture::releaseImage()
	{
		_img = nullptr;
		// TODO: is there a way to check if the image was uploaded?
	}

	TextureType Texture::getType() const
	{
		return _type;
	}

    TextureBounds TextureAtlasElement::getBounds() const
	{
		return {
			originalSize,
			originalPosition
		};
	}

	TextureBounds TextureAtlas::getBounds(std::string_view prefix) const
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

	OptionalRef<TextureAtlasElement> TextureAtlas::getElement(std::string_view name)
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

	OptionalRef<const TextureAtlasElement> TextureAtlas::getElement(std::string_view name) const
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

	ImageTextureLoader::ImageTextureLoader(IImageLoader& imgLoader)
		: _imgLoader(imgLoader)
	{
	}

	std::shared_ptr<Texture> ImageTextureLoader::operator()(std::string_view name, uint64_t flags)
	{
		auto img = _imgLoader(name);
		return Texture::create(img, name, flags);
	}

	TexturePackerTextureAtlasLoader::TexturePackerTextureAtlasLoader(IDataLoader& dataLoader, ITextureLoader& textureLoader)
		: _dataLoader(dataLoader)
		, _textureLoader(textureLoader)
	{
	}

	std::pair<int, size_t> readValueInt(std::string_view str, size_t i)
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

	std::pair<std::optional<TextureVec2>, size_t> readValueVec(std::string_view str, size_t i)
	{
		auto p = readValueInt(str, i);
		if (p.first < 0)
		{
			return { std::nullopt, std::string::npos };
		}
		TextureVec2 v = { p.first, 0 };
		i = p.second;
		if (i == std::string::npos)
		{
			return { v, i };
		}
		p = readValueInt(str, i);
		if (p.first < 0)
		{
			return { std::nullopt, std::string::npos };
		}
		v.y = p.first;
		i = p.second;
		return { v, i };
	}

	std::vector<TextureVec2> loadTextureVec2(std::string_view data)
	{
		std::vector<TextureVec2> vec;
		size_t pi = 0;
		while (pi != std::string::npos)
		{
			auto pp = readValueVec(data, pi);
			if (!pp.first.has_value())
			{
				break;
			}
			vec.push_back(pp.first.value());
			pi = pp.second;
		}
		return vec;
	}

	TextureAtlasElement loadElement(pugi::xml_node& xml)
	{
		auto positions = loadTextureVec2(xml.child("vertices").text().get());
		auto texCoords = loadTextureVec2(xml.child("verticesUV").text().get());
		std::string vertIdxVals = xml.child("triangles").text().get();
		size_t ii = 0;
		std::vector<TextureAtlasIndex> indices;
		while (ii != std::string::npos)
		{
			auto p = readValueInt(vertIdxVals, ii);
			if (p.first < 0)
			{
				break;
			}
			indices.push_back(p.first);
			ii = p.second;
		}
		return {
			xml.attribute("n").value(), positions, texCoords, indices,
			{ xml.attribute("x").as_int(), xml.attribute("y").as_int() },
			{ xml.attribute("w").as_int(), xml.attribute("h").as_int() },
			{ xml.attribute("oX").as_int(), xml.attribute("oY").as_int() },
			{ xml.attribute("oW").as_int(), xml.attribute("oH").as_int() },
			{ xml.attribute("pX").as_int(), xml.attribute("pY").as_int() },
			std::string(xml.attribute("r").value()) == "y"
		};
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

		TextureVec2 size{
			atlasXml.attribute("width").as_int(),
			atlasXml.attribute("height").as_int(),
		};

		static const char* spriteTag = "sprite";
		std::vector<TextureAtlasElement> elements;
		for (pugi::xml_node spriteXml = atlasXml.child(spriteTag); spriteXml; spriteXml = spriteXml.next_sibling(spriteTag))
		{
			elements.push_back(loadElement(spriteXml));
		}

		return std::make_shared<TextureAtlas>(texture, elements, size);
	}
}