#include <darmok/uniform.hpp>
#include <darmok/texture.hpp>
#include <darmok/utils.hpp>
#include <darmok/glm_serialize.hpp>

#include <stdexcept>

#include <glm/gtc/type_ptr.hpp>

namespace darmok
{
	UniformHandle::UniformHandle() noexcept
		: _bgfx{ bgfx::kInvalidHandle }
	{
	}

	UniformHandle::UniformHandle(const std::string& name, bgfx::UniformType::Enum type, uint16_t num) noexcept
		: _bgfx{ bgfx::createUniform(name.c_str(), type, num) }
	{
	}

	UniformHandle::~UniformHandle() noexcept
	{
		reset();
	}

	bool UniformHandle::reset() noexcept
	{
		if (isValid(_bgfx))
		{
			bgfx::destroy(_bgfx);
			_bgfx.idx = bgfx::kInvalidHandle;
			return true;
		}
		return false;
	}

	UniformHandle::operator bool() const noexcept
	{
		return valid();
	}

	bool UniformHandle::valid() const noexcept
	{
		return isValid(_bgfx);
	}

	UniformHandle::UniformHandle(UniformHandle&& other) noexcept
		: _bgfx{ other._bgfx }
	{
		other._bgfx.idx = bgfx::kInvalidHandle;
	}

	UniformHandle& UniformHandle::operator=(UniformHandle&& other) noexcept
	{
		reset();
		_bgfx = other._bgfx;
		other._bgfx.idx = bgfx::kInvalidHandle;
		return *this;
	}

	UniformHandle::operator bgfx::UniformHandle() const noexcept
	{
		return get();
	}

	const bgfx::UniformHandle& UniformHandle::get() const noexcept
	{
		return _bgfx;
	}


	UniformValue::UniformValue(const Definition& def) noexcept
	{
		if (def.has_mat4())
		{
			*this = convert<glm::mat4>(def.mat4());
		}
		else if (def.has_mat3())
		{
			*this = convert<glm::mat3>(def.mat3());
		}
		else if (def.has_vec4())
		{
			*this = convert<glm::vec4>(def.vec4());
			*this = convert<glm::vec4>(def.vec4());
		}
	}

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
		: _timeValues{ 0 }
		, _randomValues{ 0 }
		, _randomEngine{ std::random_device()() }
		, _randomDistFloat{ 0, 1 }
		, _randomDistInt{ std::numeric_limits<int>::min(), std::numeric_limits<int>::max() }
		, _timeUniform{ "u_timeVec", bgfx::UniformType::Vec4 }
		, _randomUniform{ "u_randomVec", bgfx::UniformType::Vec4 }
	{
	}

	void BasicUniforms::clear() noexcept
	{
		_timeValues = glm::vec4{ 0 };
	}

	void BasicUniforms::update(float deltaTime) noexcept
	{
		_timeValues.x += deltaTime;
		++_timeValues.y;
		// TODO: probably very slow
		_randomValues = glm::vec4{
			_randomDistFloat(_randomEngine),
			_randomDistFloat(_randomEngine),
			_randomDistFloat(_randomEngine),
			_randomDistFloat(_randomEngine)
		};
	}

	void BasicUniforms::configure(bgfx::Encoder& encoder) const noexcept
	{
		encoder.setUniform(_timeUniform, glm::value_ptr(_timeValues));
		encoder.setUniform(_randomUniform, glm::value_ptr(_randomValues));
	}

	void UniformHandleContainer::clear() noexcept
	{
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
		auto& handle = getHandle({ key.name(), bgfx::UniformType::Sampler });
		encoder.setTexture(key.stage(), handle, tex->getHandle());
	}

	void UniformHandleContainer::configure(bgfx::Encoder& encoder, const std::string& name, const UniformValue& val) const noexcept
	{
		auto& handle = getHandle({ name, val.getType() });
		encoder.setUniform(handle, val.ptr());
	}

	const UniformHandle& UniformHandleContainer::getHandle(const Key& key) const noexcept
	{
		auto itr = _handles.find(key);
		if (itr == _handles.end())
		{
			itr = _handles.emplace(key.name, UniformHandle{ key.name, key.type }).first;
		}
		return itr->second;
	}
}