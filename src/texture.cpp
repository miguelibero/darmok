#include "texture.hpp"

namespace darmok
{
	Texture::Texture(const bgfx::TextureHandle& handle, TextureType type, std::shared_ptr<Image> img) noexcept
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

	std::shared_ptr<Texture> Texture::create(const TextureCreationConfig& cfg, uint64_t flags) noexcept
	{
		bgfx::TextureHandle handle{ bgfx::kInvalidHandle };

		switch (cfg.type)
		{
		case TextureType::CubeMap:
			handle = bgfx::createTextureCube(cfg.size.x, cfg.mips, cfg.layers, cfg.format, flags);
			break;
		case TextureType::Texture2D:
			handle = bgfx::createTexture2D(cfg.size.x, cfg.size.y, cfg.mips, cfg.layers, cfg.format, flags);
			break;
		case TextureType::Texture3D:
			handle = bgfx::createTexture3D(cfg.size.x, cfg.size.y, cfg.depth, cfg.mips, cfg.format, flags);
			break;
		default:
			return nullptr;
		}

		if (!bgfx::isValid(handle))
		{
			return nullptr;
		}

		return std::make_shared<Texture>(handle, cfg.type);
	}

	std::shared_ptr<Texture> Texture::create(std::shared_ptr<Image> img, uint64_t flags) noexcept
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

		return std::make_shared<Texture>(handle, type, img);
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

	Texture& Texture::setName(std::string_view name) noexcept
	{
		bgfx::setName(_handle, name.data(), name.size());
		return *this;
	}

	ImageTextureLoader::ImageTextureLoader(IImageLoader& imgLoader) noexcept
		: _imgLoader(imgLoader)
	{
	}

	std::shared_ptr<Texture> ImageTextureLoader::operator()(std::string_view name, uint64_t flags) noexcept
	{
		auto tex = Texture::create(_imgLoader(name), flags);
		if (tex != nullptr)
		{
			tex->setName(name);
		}
		return tex;
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