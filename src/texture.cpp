#include "texture.hpp"
#include <glm/gtx/string_cast.hpp>

namespace darmok
{
	Texture::Texture(const bgfx::TextureHandle& handle, const Config& cfg) noexcept
		: _handle(handle)
		, _config(cfg)
	{
	}

	Texture::Texture(const Image& img, uint64_t flags) noexcept
		: _handle{ bgfx::kInvalidHandle }
		, _config{}
	{

		_config.format = bgfx::TextureFormat::Enum(img.getFormat());
		_config.mips = img.getMipCount() > 1;
		_config.layers = img.getLayerCount();
		_config.type = img.getTextureType(flags);
		_config.size = img.getSize();
		_config.depth = uint16_t(img.getDepth());

		if (_config.type == TextureType::Unknown)
		{
			// TODO: maybe throw here
			return;
		}

		// copying the memory of the image becauyse bgfx needs to maintain the memory for some frames
		// since the texture creation can b e async, and it could happen that the std::shared_ptr<Image>
		// is destroyed before (for example if a texture is created and replaced in the same frame
		const auto mem = img.makeCopyRef();
		auto w = uint16_t(_config.size.x);
		auto h = uint16_t(_config.size.y);

		switch (_config.type)
		{
		case TextureType::CubeMap:
			_handle = bgfx::createTextureCube(w, _config.mips, _config.layers, _config.format, flags, mem);
			break;
		case TextureType::Texture3D:
			_handle = bgfx::createTexture3D(w, h, _config.depth, _config.mips, _config.format, flags, mem);
			break;
		case TextureType::Texture2D:
			_handle = bgfx::createTexture2D(w, h, _config.mips, _config.layers, _config.format, flags, mem);
			break;
		}
	}

	Texture::Texture(const Config& cfg, uint64_t flags) noexcept
		: _handle{ bgfx::kInvalidHandle }
		, _config(cfg)
	{
		bgfx::TextureHandle handle{ bgfx::kInvalidHandle };
		switch (cfg.type)
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
		}
	}

	Texture::~Texture() noexcept
	{
		if (isValid(_handle))
		{
			bgfx::destroy(_handle);
		}
	}

	std::string TextureConfig::to_string() const noexcept
	{
		return "size:" + glm::to_string(size);
	}

	std::string Texture::to_string() const noexcept
	{
		return "Texture(" + std::to_string(_handle.idx)
			+ " " + _config.to_string() + ")";
	}

	const bgfx::TextureHandle& Texture::getHandle() const noexcept
	{
		return _handle;
	}

	const glm::uvec2& Texture::getSize() const noexcept
	{
		return _config.size;
	}

	TextureType Texture::getType() const noexcept
	{
		return _config.type;
	}

	bgfx::TextureFormat::Enum Texture::getFormat() const noexcept
	{
		return _config.format;
	}

	uint16_t Texture::getLayerCount() const noexcept
	{
		return _config.layers;
	}

	uint16_t Texture::getDepth() const noexcept
	{
		return _config.depth;
	}

	bool Texture::hasMips() const noexcept
	{
		return _config.mips;
	}

	Texture& Texture::setName(std::string_view name) noexcept
	{
		if (isValid(_handle))
		{
			bgfx::setName(_handle, name.data(), name.size());
		}
		return *this;
	}

	ImageTextureLoader::ImageTextureLoader(IImageLoader& imgLoader) noexcept
		: _imgLoader(imgLoader)
	{
	}

	std::shared_ptr<Texture> ImageTextureLoader::operator()(std::string_view name, uint64_t flags) noexcept
	{
		auto tex = std::make_shared<Texture>(*_imgLoader(name), flags);
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
		auto tex = std::make_shared<Texture>(*Image::create(_alloc, color, _size));
		_cache.emplace(color, tex);
		return tex;
	}

}