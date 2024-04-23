#include "texture.hpp"

namespace darmok
{
	Texture::Texture(std::shared_ptr<Image> img, uint64_t flags) noexcept
		: _img(img)
		, _flags(flags)
		, _handle{ bgfx::kInvalidHandle }
		, _type(TextureType::Unknown)
	{
		// we're delaying the handle creation because bgfx tries to load the image asyncronously (at least in DX11)
		// that produces that if a texture is added and then removed, bgfx crashes afterwards trying to load the image (that was already released)
		// TODO: check if there is a better way
		if (img->isCubeMap())
		{
			_type = TextureType::CubeMap;
		}
		else if (1 < img->getDepth())
		{
			_type = TextureType::Texture3D;
		}
		else
		{
			auto format = bgfx::TextureFormat::Enum(_img->getFormat());
			auto layers = _img->getLayerCount();
			if (bgfx::isTextureValid(0, false, layers, format, _flags))
			{
				_type = TextureType::Texture2D;
			}
		}
	}

	Texture::~Texture() noexcept
	{
		if (isValid(_handle))
		{
			bgfx::destroy(_handle);
		}
	}

	void Texture::load() noexcept
	{
		if (isValid(_handle) || _img == nullptr || _img->empty())
		{
			return;
		}

		const auto mem = _img->makeRef();
		auto format = bgfx::TextureFormat::Enum(_img->getFormat());
		auto hasMips = 1 < _img->getMipCount();
		auto s = _img->getSize();
		auto w = uint16_t(s.x);
		auto h = uint16_t(s.y);
		auto layers = _img->getLayerCount();

		bgfx::TextureHandle handle{ bgfx::kInvalidHandle };
		TextureType type = TextureType::Unknown;

		if (_type == TextureType::CubeMap)
		{
			_handle = bgfx::createTextureCube(w, hasMips, layers, format, _flags, mem);
		}
		else if (_type == TextureType::Texture3D)
		{
			_handle = bgfx::createTexture3D(w, h, uint16_t(_img->getDepth()), hasMips, format, _flags, mem);
		}
		else if (_type == TextureType::Texture2D)
		{
			_handle = bgfx::createTexture2D(w, h, hasMips, layers, format, _flags, mem);
		}
		if (isValid(_handle) && !_name.empty())
		{
			bgfx::setName(_handle, _name.data(), _name.size());
		}
	}

	bgfx::TextureHandle Texture::getHandle() noexcept
	{
		load();
		return _handle;
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
		if (isValid(_handle))
		{
			bgfx::setName(_handle, name.data(), name.size());
		}
		else
		{
			_name = name;
		}
		return *this;
	}

	ImageTextureLoader::ImageTextureLoader(IImageLoader& imgLoader) noexcept
		: _imgLoader(imgLoader)
	{
	}

	RenderTexture::RenderTexture(const Config& cfg, uint64_t flags) noexcept
		: _handle{ bgfx::kInvalidHandle }
		, _type(cfg.type)
	{
		switch (_type)
		{
		case TextureType::CubeMap:
			_handle = bgfx::createTextureCube(cfg.size.x, cfg.mips, cfg.layers, cfg.format, flags);
			break;
		case TextureType::Texture2D:
			_handle = bgfx::createTexture2D(cfg.size.x, cfg.size.y, cfg.mips, cfg.layers, cfg.format, flags);
			break;
		case TextureType::Texture3D:
			_handle = bgfx::createTexture3D(cfg.size.x, cfg.size.y, cfg.depth, cfg.mips, cfg.format, flags);
			break;
		default:
			break;
		}
	}

	RenderTexture::~RenderTexture() noexcept
	{
		if (isValid(_handle))
		{
			bgfx::destroy(_handle);
		}
	}

	bgfx::TextureHandle RenderTexture::getHandle() noexcept
	{
		return _handle;
	}

	const bgfx::TextureHandle& RenderTexture::getHandle() const noexcept
	{
		return _handle;
	}

	TextureType RenderTexture::getType() const noexcept
	{
		return _type;
	}

	RenderTexture& RenderTexture::setName(std::string_view name) noexcept
	{
		bgfx::setName(_handle, name.data(), name.size());
		return *this;
	}

	std::shared_ptr<Texture> ImageTextureLoader::operator()(std::string_view name, uint64_t flags) noexcept
	{
		auto tex = std::make_shared<Texture>(_imgLoader(name), flags);
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
		auto tex = std::make_shared<Texture>(Image::create(_alloc, color, _size));
		_cache.emplace(color, tex);
		return tex;
	}

}