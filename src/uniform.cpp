#include <darmok/uniform.hpp>
#include <darmok/texture.hpp>
#include <darmok/utils.hpp>

#include <stdexcept>

#include <glm/gtc/type_ptr.hpp>

namespace darmok
{
	UniformValue::UniformValue(const Variant& val) noexcept
		: _value(val)
	{
	}

	UniformValue::UniformValue(Variant&& val) noexcept
		: _value(std::move(val))
	{
	}

	UniformValue& UniformValue::operator=(const Variant& val) noexcept
	{
		_value = val;
		return *this;
	}

	UniformValue& UniformValue::operator=(Variant&& val) noexcept
	{
		_value = std::move(val);
		return *this;
	}


	bgfx::UniformType::Enum UniformValue::getType() const noexcept
	{
		if (std::holds_alternative<glm::vec4>(_value))
		{
			return bgfx::UniformType::Vec4;
		}
		else if (std::holds_alternative<glm::mat3>(_value))
		{
			return bgfx::UniformType::Mat3;
		}
		else if (std::holds_alternative<glm::mat4>(_value))
		{
			return bgfx::UniformType::Mat4;
		}
		return bgfx::UniformType::Count;
	}

	const void* UniformValue::ptr() const noexcept
	{
		return std::visit([](const auto& value) {
			return glm::value_ptr(value);
		}, _value);
	}
	
	BasicUniforms::BasicUniforms() noexcept
		: _timeValues(0)
		, _randomValues(0)
		, _randomEngine(std::random_device()())
		, _randomDistFloat(0, 1)
		, _randomDistInt(std::numeric_limits<int>::min(), std::numeric_limits<int>::max())
		, _timeUniform{ bgfx::kInvalidHandle }
		, _randomUniform{ bgfx::kInvalidHandle }
	{
	}

	BasicUniforms::~BasicUniforms() noexcept
	{
		shutdown();
	}

	void BasicUniforms::init() noexcept
	{
		_timeValues = glm::vec4(0);
		if (isValid(_timeUniform))
		{
			bgfx::destroy(_timeUniform);
		}
		_timeUniform = bgfx::createUniform("u_timeVec", bgfx::UniformType::Vec4);
		if (isValid(_randomUniform))
		{
			bgfx::destroy(_randomUniform);
		}
		_randomUniform = bgfx::createUniform("u_randomVec", bgfx::UniformType::Vec4);
	}

	void BasicUniforms::shutdown() noexcept
	{
		std::vector<std::reference_wrapper<bgfx::UniformHandle>> uniforms = {
			_timeUniform, _randomUniform
		};
		for (auto& uniform : uniforms)
		{
			if (isValid(uniform))
			{
				bgfx::destroy(uniform);
				uniform.get().idx = bgfx::kInvalidHandle;
			}
		}
	}

	void BasicUniforms::update(float deltaTime) noexcept
	{
		_timeValues.x += deltaTime;
		++_timeValues.y;
		// TODO: probably very slow
		_randomValues = glm::vec4(
			_randomDistFloat(_randomEngine),
			_randomDistFloat(_randomEngine),
			_randomDistFloat(_randomEngine),
			_randomDistFloat(_randomEngine)
		);
	}

	void BasicUniforms::configure(bgfx::Encoder& encoder) const noexcept
	{
		encoder.setUniform(_timeUniform, glm::value_ptr(_timeValues));
		encoder.setUniform(_randomUniform, glm::value_ptr(_randomValues));
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

	UniformHandleContainer::~UniformHandleContainer() noexcept
	{
		shutdown();
	}

	void UniformHandleContainer::shutdown() noexcept
	{
		for (auto& [key, handle] : _handles)
		{
			bgfx::destroy(handle);
		}
		_handles.clear();
	}

	bool UniformHandleContainer::Key::operator==(const UniformHandleContainer::Key& other) const noexcept
	{
		return name == other.name && type == other.type;
	}

	bool UniformHandleContainer::Key::operator!=(const UniformHandleContainer::Key& other) const noexcept
	{
		return !operator==(other);
	}

	size_t UniformHandleContainer::Key::hash() const noexcept
	{
		size_t hash = 0;
		hashCombine(hash, name, type);
		return hash;
	}

	void UniformHandleContainer::configure(bgfx::Encoder& encoder, const UniformValueMap& values) const
	{
		for (const auto& [name, val] : values)
		{
			configure(encoder, name, val);
		}
	}

	void UniformHandleContainer::configure(bgfx::Encoder& encoder, const UniformTextureMap& textures) const
	{
		for (const auto& [key, tex] : textures)
		{
			configure(encoder, key, tex);
		}
	}

	void UniformHandleContainer::configure(bgfx::Encoder& encoder, const TextureUniformKey& key, const std::shared_ptr<Texture>& tex) const noexcept
	{
		auto handle = getHandle({ key.name, bgfx::UniformType::Sampler });
		encoder.setTexture(key.stage, handle, tex->getHandle());
	}

	void UniformHandleContainer::configure(bgfx::Encoder& encoder, const std::string& name, const UniformValue& val) const noexcept
	{
		auto handle = getHandle({ name, val.getType() });
		encoder.setUniform(handle, val.ptr());
	}

	bgfx::UniformHandle UniformHandleContainer::getHandle(const Key& key) const noexcept
	{
		auto itr = _handles.find(key);
		if (itr == _handles.end())
		{
			auto handler = bgfx::createUniform(key.name.c_str(), bgfx::UniformType::Sampler);
			itr = _handles.emplace(key.name, handler).first;
		}
		return itr->second;
	}
}