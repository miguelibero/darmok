#include <darmok/uniform.hpp>
#include <stdexcept>
#include <glm/gtc/type_ptr.hpp>

namespace darmok
{
	Uniform::Uniform() noexcept
		: _type(bgfx::UniformType::Vec4)
		, _handle{ bgfx::kInvalidHandle }
	{
	}

	Uniform::Uniform(const std::string& name, bgfx::UniformType::Enum type, bool autoInit) noexcept
		: _name(name)
		, _type(type)
		, _handle{ bgfx::kInvalidHandle }
	{
		if (autoInit)
		{
			init();
		}
	}

	Uniform::Uniform(const std::string& name, const Value& value, bool autoInit) noexcept
		: _name(name)
		, _type(getType(value))
		, _value(value)
		, _handle{ bgfx::kInvalidHandle }
	{
		if (autoInit)
		{
			init();
		}
	}

	Uniform::~Uniform() noexcept
	{
		shutdown();
	}

	Uniform::Uniform(const Uniform& other) noexcept
		: _name(other._name)
		, _type(other._type)
		, _value(other._value)
		, _handle{ bgfx::kInvalidHandle }
	{
		if (isValid(other._handle))
		{
			init();
		}
	}

	Uniform& Uniform::operator=(const Uniform& other) noexcept
	{
		if (_handle.idx == other._handle.idx)
		{
			return *this;
		}
		shutdown();

		_name = other._name;
		_type = other._type;
		_value = other._value;

		if (isValid(other._handle))
		{
			init();
		}

		return *this;
	}

	Uniform::Uniform(Uniform&& other) noexcept
		: _name(other._name)
		, _type(other._type)
		, _value(other._value)
		, _handle(other._handle)
	{
		other._handle.idx = bgfx::kInvalidHandle;
	}

	Uniform& Uniform::operator=(Uniform&& other) noexcept
	{
		if (_handle.idx == other._handle.idx)
		{
			return *this;
		}
		shutdown();

		_name = other._name;
		_type = other._type;
		_value = other._value;
		_handle = other._handle;

		other._handle.idx = bgfx::kInvalidHandle;
		return *this;
	}

	void Uniform::init() noexcept
	{
		if (isValid(_handle))
		{
			shutdown();
		}
		_handle = bgfx::createUniform(_name.c_str(), _type);
	}

	void Uniform::shutdown() noexcept
	{
		if (isValid(_handle))
		{
			bgfx::destroy(_handle);
			_handle.idx = bgfx::kInvalidHandle;
		}
	}

	bool Uniform::operator==(const Uniform& other) const noexcept
	{
		return _name == other._name && _type == other._type
			&& _handle.idx == other._handle.idx && _value == other._value;
	}

	bool Uniform::operator!=(const Uniform& other) const noexcept
	{
		return !operator==(other);
	}

	Uniform& Uniform::operator=(const UniformValue& value) noexcept
	{
		_value = value;
		return *this;
	}

	Uniform::operator const UniformValue& () const noexcept
	{
		return get();
	}

	Uniform::operator const glm::vec4& () const
	{
		if (auto vec4 = std::get_if<glm::vec4>(&_value))
		{
			return *vec4;
		}
		throw std::invalid_argument("uniform is not a vec4");
	}

	Uniform::operator const glm::mat3& () const
	{
		if (auto mat3 = std::get_if<glm::mat3>(&_value))
		{
			return *mat3;
		}
		throw std::invalid_argument("uniform is not a mat3");
	}

	Uniform::operator const glm::mat4& () const
	{
		if (auto mat4 = std::get_if<glm::mat4>(&_value))
		{
			return *mat4;
		}
		throw std::invalid_argument("uniform is not a mat4");
	}

	bgfx::UniformType::Enum Uniform::getType(const Value& value) noexcept
	{
		if (std::holds_alternative<glm::vec4>(value))
		{
			return bgfx::UniformType::Vec4;
		}
		else if (std::holds_alternative<glm::mat3>(value))
		{
			return bgfx::UniformType::Mat3;
		}
		else if (std::holds_alternative<glm::mat4>(value))
		{
			return bgfx::UniformType::Mat4;
		}
		return bgfx::UniformType::Count;
	}

	Uniform& Uniform::setType(bgfx::UniformType::Enum type) noexcept
	{
		if (_type != type)
		{
			auto initialized = isValid(_handle);
			if (initialized)
			{
				shutdown();
			}
			_type = type;
			if (initialized)
			{
				init();
			}
		}
		return *this;
	}

	Uniform& Uniform::set(const Value& value) noexcept
	{
		setType(getType(value));
		_value = value;
		return *this;
	}

	const UniformValue& Uniform::get() const noexcept
	{
		return _value;
	}

	const Uniform& Uniform::configure(bgfx::Encoder& encoder) const
	{
		if (!isValid(_handle))
		{
			throw std::runtime_error("uniform not initialized");
		}
		doConfigure(encoder);
		return *this;
	}

	Uniform& Uniform::configure(bgfx::Encoder& encoder) noexcept
	{
		if (!isValid(_handle))
		{
			init();
		}
		doConfigure(encoder);
		return *this;
	}

	void Uniform::doConfigure(bgfx::Encoder& encoder) const noexcept
	{
		if (auto vec4 = std::get_if<glm::vec4>(&_value))
		{
			encoder.setUniform(_handle, glm::value_ptr(*vec4));
		}
		else if (auto mat3 = std::get_if<glm::mat3>(&_value))
		{
			encoder.setUniform(_handle, glm::value_ptr(*mat3));
		}
		else if (auto mat4 = std::get_if<glm::mat4>(&_value))
		{
			encoder.setUniform(_handle, glm::value_ptr(*mat4));
		}
	}

	UniformContainer::UniformContainer(bool autoInit) noexcept
		: _autoInit(autoInit)
		, _initialized(autoInit)
	{
	}

	UniformContainer::~UniformContainer() noexcept
	{
		shutdown();
	}

	void UniformContainer::init() noexcept
	{
		_initialized = true;
		for (auto& [name, uniform] : _uniforms)
		{
			uniform.init();
		}
	}

	void UniformContainer::shutdown() noexcept
	{
		_initialized = false;
		for (auto& [name, uniform] : _uniforms)
		{
			uniform.shutdown();
		}
	}

	UniformContainer& UniformContainer::set(const std::string& name, std::optional<UniformValue> value) noexcept
	{
		auto itr = _uniforms.find(name);
		if (value)
		{
			if (itr == _uniforms.end())
			{
				auto autoInit = _autoInit || _initialized;
				_uniforms.emplace(name, Uniform(name, value.value(), autoInit));
			}
			else
			{
				itr->second.set(value.value());
			}
		}
		else if (itr != _uniforms.end())
		{
			_uniforms.erase(itr);
		}
		return *this;
	}

	const UniformContainer& UniformContainer::configure(bgfx::Encoder& encoder) const
	{
		for (auto& [name, uniform] : _uniforms)
		{
			uniform.configure(encoder);
		}
		return *this;
	}

	UniformContainer& UniformContainer::configure(bgfx::Encoder& encoder) noexcept
	{
		for (auto& [name, uniform] : _uniforms)
		{
			uniform.configure(encoder);
		}
		return *this;
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

}