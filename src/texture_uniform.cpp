#include <darmok/texture_uniform.hpp>
#include <darmok/texture.hpp>
#include <darmok/utils.hpp>
#include <filesystem>

namespace darmok
{
    
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

	TextureUniform::TextureUniform() noexcept
		: TextureUniform({ "", 0 }, nullptr)
	{
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

	TextureUniform::TextureUniform(const Key& key, std::shared_ptr<Texture> tex, bool autoInit) noexcept
		: _key(key)
		, _texture(std::move(tex))
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
		const TextureUniformKey key{ name, stage };
		auto autoInit = _autoInit || _initialized;
		_uniforms.emplace(key, TextureUniform(key, texture, autoInit));
		return *this;
	}

	const TextureUniformContainer& TextureUniformContainer::configure(bgfx::Encoder& encoder) const
	{
		for (const auto& [key, uniform] : _uniforms)
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