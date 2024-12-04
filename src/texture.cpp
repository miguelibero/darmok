#include <darmok/texture.hpp>
#include <darmok/data.hpp>
#include <darmok/utils.hpp>
#include <glm/gtx/string_cast.hpp>
#include <stdexcept>

namespace darmok
{
	TextureDefinition TextureDefinition::fromImage(const Image& img, uint64_t loadFlags) noexcept
	{
		return { img.getData(), img.getTextureConfig(loadFlags), loadFlags };
	}

	Texture::Texture(const bgfx::TextureHandle& handle, const Config& cfg) noexcept
		: _handle(handle)
		, _config(cfg)
	{
	}

	Texture::Texture(const Image& img, uint64_t flags) noexcept
		: Texture(img.getData(), img.getTextureConfig(flags), flags)
	{
	}

	Texture::Texture(const DataView& data, const Config& cfg, uint64_t flags) noexcept
		: _handle{ bgfx::kInvalidHandle }
		, _config(cfg)
	{
		if (_config.type == TextureType::Unknown)
		{
			// TODO: maybe throw here
			return;
		}

		// copying the memory of the image becauyse bgfx needs to maintain the memory for some frames
		// since the texture creation can b e async, and it could happen that the std::shared_ptr<Image>
		// is destroyed before (for example if a texture is created and replaced in the same frame
		const auto mem = data.copyMem();
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

	Texture::Texture(const TextureDefinition& def) noexcept
		: Texture(def.data, def.config, def.loadFlags)
	{
	}

	Texture::Texture(Texture&& other) noexcept
		: _handle(other._handle)
		, _config(other._config)
	{
		other._handle.idx = bgfx::kInvalidHandle;
		other._config = Config::getEmpty();
	}

	Texture& Texture::operator=(Texture&& other) noexcept
	{
		_handle = other._handle;
		_config = other._config;
		other._handle.idx = bgfx::kInvalidHandle;
		other._config = Config::getEmpty();
		return *this;
	}

	Texture::~Texture() noexcept
	{
		if (isValid(_handle))
		{
			bgfx::destroy(_handle);
		}
	}

	const TextureConfig& TextureConfig::getEmpty() noexcept
	{
		static TextureConfig empty{
			glm::uvec2(0),
			bgfx::TextureFormat::Unknown,
			TextureType::Unknown,
			0, false, 0
		};
		return empty;
	}

	std::string TextureConfig::toString() const noexcept
	{
		return "TextureConfig(" + glm::to_string(size) + ")";
	}

	bgfx::TextureInfo TextureConfig::getInfo() const noexcept
	{
		bgfx::TextureInfo info;
		auto cubeMap = type == TextureType::CubeMap;
		bgfx::calcTextureSize(info, size.x, size.y, depth, cubeMap, mips, layers, format);
		return info;
	}

	uint32_t Texture::getStorageSize() const noexcept
	{
		return _config.getInfo().storageSize;
	}

	uint8_t Texture::getMipsCount() const noexcept
	{
		return _config.getInfo().numMips;
	}

	uint8_t Texture::getBitsPerPixel() const noexcept
	{
		return _config.getInfo().bitsPerPixel;
	}

	void Texture::update(const DataView& data, uint8_t mip)
	{
		if (_config.type == TextureType::Texture3D)
		{
			update(data, glm::uvec3(getSize(), getDepth()), glm::uvec3(0), mip);
		}
		else
		{
			update(data, getSize(), glm::uvec2(0), mip);
		}
	}

	void Texture::update(const DataView& data, const glm::uvec2& size, const glm::uvec2& origin, uint8_t mip, uint16_t layer, uint8_t side)
	{
		if (_config.type == TextureType::Texture3D)
		{
			throw std::runtime_error("does not work on 3D textures");
		}
		auto memSize = size.x * size.y * getBitsPerPixel() / 8;
		if (memSize == 0)
		{
			return;
		}
		if (data.size() < memSize)
		{
			throw std::runtime_error("data is smaller that expected size");
		}
		if (_config.type == TextureType::Texture2D)
		{
			bgfx::updateTexture2D(_handle, layer, mip,
				origin.x, origin.y, size.x, size.y,
				data.copyMem(0, memSize));
		}
		else
		{
			bgfx::updateTextureCube(_handle, layer, side, mip,
				origin.x, origin.y, size.x, size.y,
				data.copyMem(0, memSize));
		}
	}

	void Texture::update(const DataView& data, const glm::uvec3& size, const glm::uvec3& origin, uint8_t mip)
	{
		if (_config.type != TextureType::Texture3D)
		{
			throw std::runtime_error("does only work on 3D textures");
		}
		auto memSize = size.x * size.y * size.z * getBitsPerPixel() / 8;
		if (memSize == 0)
		{
			return;
		}
		if (data.size() < memSize)
		{
			throw std::runtime_error("data is smaller that expected size");
		}
		bgfx::updateTexture3D(_handle, mip,
			origin.x, origin.y, origin.y, size.x, size.y, size.z,
			data.copyMem(0, memSize));

	}

	uint32_t Texture::read(Data& data) noexcept
	{
		auto size = getStorageSize();
		if (data.size() < size)
		{
			data.resize(size);
		}
		return bgfx::readTexture(_handle, data.ptr());
	}

	std::string Texture::toString() const noexcept
	{
		return "Texture(" + std::to_string(_handle.idx)
			+ " " + _config.toString() + ")";
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
			bgfx::setName(_handle, name.data(), int32_t(name.size()));
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

	bool TextureUniformKey::operator==(const TextureUniformKey& other) const noexcept
	{
		return name == other.name && stage == other.stage;
	}

	bool TextureUniformKey::operator!=(const TextureUniformKey& other) const noexcept
	{
		return !operator==(other);
	}

	size_t TextureUniformKey::hash() const noexcept
	{
		size_t hash = 0;
		hashCombine(hash, name, stage);
		return hash;
	}

	TextureUniform::TextureUniform(const std::string& name, uint8_t stage, bool autoInit) noexcept
		: TextureUniform({ name, stage }, nullptr, autoInit)
	{

	}

	TextureUniform::TextureUniform(const std::string& name, uint8_t stage, const std::shared_ptr<Texture>& tex, bool autoInit) noexcept
		: TextureUniform({ name, stage }, tex, autoInit)
	{

	}

	TextureUniform::TextureUniform(const Key& key, bool autoInit) noexcept
		: TextureUniform(key, nullptr, autoInit)
	{
	}

	TextureUniform::TextureUniform(const Key& key, const std::shared_ptr<Texture>& tex, bool autoInit) noexcept
		: _key(key)
		, _texture(tex)
		, _handle{ bgfx::kInvalidHandle }
	{
		if (autoInit)
		{
			init();
		}
	}

	TextureUniform::~TextureUniform() noexcept
	{
		shutdown();
	}

	TextureUniform::TextureUniform(const TextureUniform& other) noexcept
		: _key(other._key)
		, _texture(other._texture)
		, _handle{ bgfx::kInvalidHandle }
	{
		if (isValid(other._handle))
		{
			init();
		}
	}

	TextureUniform& TextureUniform::operator=(const TextureUniform& other) noexcept
	{
		if (_handle.idx == other._handle.idx)
		{
			return *this;
		}

		shutdown();

		_key = other._key;
		_texture = other._texture;

		if (isValid(other._handle))
		{
			init();
		}
		return *this;
	}

	TextureUniform::TextureUniform(TextureUniform&& other) noexcept
		: _key(other._key)
		, _texture(other._texture)
		, _handle(other._handle)
	{
		other._texture.reset();
		other._handle.idx = bgfx::kInvalidHandle;
	}
	
	TextureUniform& TextureUniform::operator=(TextureUniform&& other) noexcept
	{
		if (_handle.idx == other._handle.idx)
		{
			return *this;
		}

		shutdown();

		_key = other._key;
		_texture = other._texture;
		_handle = other._handle;

		other._texture.reset();
		other._handle.idx = bgfx::kInvalidHandle;
		return *this;
	}

	void TextureUniform::init() noexcept
	{
		if (isValid(_handle))
		{
			shutdown();
		}
		_handle = bgfx::createUniform(_key.name.c_str(), bgfx::UniformType::Sampler);
	}

	void TextureUniform::shutdown() noexcept
	{
		if (isValid(_handle))
		{
			bgfx::destroy(_handle);
			_handle.idx = bgfx::kInvalidHandle;
		}
	}

	bool TextureUniform::operator==(const TextureUniform& other) const noexcept
	{
		return _key == other._key && _texture == other._texture;
	}

	bool TextureUniform::operator!=(const TextureUniform& other) const noexcept
	{
		return !operator==(other);
	}

	TextureUniform& TextureUniform::operator=(const std::shared_ptr<Texture>& tex) noexcept
	{
		set(tex);
		return *this;
	}

	TextureUniform::operator const std::shared_ptr<Texture>& () const noexcept
	{
		return get();
	}

	TextureUniform& TextureUniform::set(const std::shared_ptr<Texture>& texture) noexcept
	{
		_texture = texture;
		return *this;
	}

	const std::shared_ptr<Texture>& TextureUniform::get() const noexcept
	{
		return _texture;
	}

	const TextureUniform& TextureUniform::configure(bgfx::Encoder& encoder) const
	{
		if (!isValid(_handle))
		{
			throw std::runtime_error("texture uniform not initialized");
		}
		doConfigure(encoder);
		return *this;
	}

	TextureUniform& TextureUniform::configure(bgfx::Encoder& encoder) noexcept
	{
		if (!isValid(_handle))
		{
			init();
		}
		doConfigure(encoder);
		return *this;
	}

	void TextureUniform::doConfigure(bgfx::Encoder& encoder) const noexcept
	{
		encoder.setTexture(_key.stage, _handle, _texture->getHandle());

	}

	TextureUniformContainer::TextureUniformContainer(bool autoInit) noexcept
		: _autoInit(autoInit)
		, _initialized(autoInit)
	{
	}

	TextureUniformContainer::~TextureUniformContainer() noexcept
	{
		shutdown();
	}

	void TextureUniformContainer::init() noexcept
	{
		_initialized = true;
		for (auto& [name, uniform] : _uniforms)
		{
			uniform.init();
		}
	}

	void TextureUniformContainer::shutdown() noexcept
	{
		_initialized = false;
		for (auto& [name, uniform] : _uniforms)
		{
			uniform.shutdown();
		}
	}

	TextureUniformContainer& TextureUniformContainer::set(const std::string& name, uint8_t stage, const std::shared_ptr<Texture>& texture) noexcept
	{
		TextureUniformKey key{ name, stage };
		auto autoInit = _autoInit || _initialized;
		_uniforms.emplace(key, TextureUniform(key, texture, autoInit));
		return *this;
	}

	const TextureUniformContainer& TextureUniformContainer::configure(bgfx::Encoder& encoder) const
	{
		for (auto& [key, uniform] : _uniforms)
		{
			uniform.configure(encoder);
		}
		return *this;
	}

	TextureUniformContainer& TextureUniformContainer::configure(bgfx::Encoder& encoder) noexcept
	{
		for (auto& [key, uniform] : _uniforms)
		{
			uniform.configure(encoder);
		}
		return *this;
	}
}