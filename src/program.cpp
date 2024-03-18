#include "program.hpp"
#include <darmok/data.hpp>
#include <unordered_map>
#include <glm/gtc/type_ptr.hpp>

namespace darmok
{
    Program::Program(const bgfx::ProgramHandle& handle) noexcept
		: _handle(handle)
	{
	}
    Program::~Program() noexcept
    {
		if (isValid(_handle))
		{
			bgfx::destroy(_handle);
		}
    }

	const bgfx::ProgramHandle& Program::getHandle() const noexcept
	{
		return _handle;
	}  

	DataProgramLoader::DataProgramLoader(IDataLoader& dataLoader) noexcept
		: _dataLoader(dataLoader)
	{
	}

	std::string DataProgramLoader::getShaderExt()
	{
		switch (bgfx::getRendererType())
		{
		case bgfx::RendererType::Noop:
			return "";
		case bgfx::RendererType::Direct3D11:
		case bgfx::RendererType::Direct3D12:
			return ".dx11";
		case bgfx::RendererType::Agc:
		case bgfx::RendererType::Gnm:
			return ".pssl";
		case bgfx::RendererType::Metal:
			return ".metal";
		case bgfx::RendererType::Nvn:
			return ".nvn";
		case bgfx::RendererType::OpenGL:
			return ".glsl";
		case bgfx::RendererType::OpenGLES:
			return ".essl";
		case bgfx::RendererType::Vulkan:
			return ".spv";
		}
		throw std::runtime_error("unknown renderer type");
	}

	bgfx::ShaderHandle DataProgramLoader::loadShader(const std::string& filePath)
	{
		std::string dataName = filePath + getShaderExt() + ".bin";
		auto data = _dataLoader(dataName);
		if (data == nullptr || data->empty())
		{
			throw std::runtime_error("got empty data");
		}
		bgfx::ShaderHandle handle = bgfx::createShader(data->makeRef());
		bgfx::setName(handle, filePath.c_str());
		return handle;
	}

	std::shared_ptr<Program> DataProgramLoader::operator()(std::string_view vertexName, std::string_view fragmentName)
	{
		const bgfx::ShaderHandle vsh = loadShader(std::string(fragmentName));
		bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
		if (!fragmentName.empty())
		{
			fsh = loadShader(std::string(fragmentName));
		}
		auto handle = bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
		return std::make_shared<Program>(handle);
	}

	bool ProgramAttribDefinition::operator==(const ProgramAttribDefinition& other) const noexcept
	{
		return type == other.type && num == other.num && normalize == other.normalize;
	}

	bool ProgramAttribDefinition::operator!=(const ProgramAttribDefinition& other) const noexcept
	{
		return !operator==(other);
	}

	ProgramUniformDefinition::ProgramUniformDefinition(std::string name, bgfx::UniformType::Enum type, uint16_t num) noexcept
		: _name(std::move(name))
		, _type(type)
		, _num(num)
	{
	}

	ProgramUniformDefinition::ProgramUniformDefinition(std::string name, std::shared_ptr<Texture>& defaultValue, uint16_t num) noexcept
		: _name(std::move(name))
		, _type(bgfx::UniformType::Sampler)
		, _num(num)
		, _defaultTexture(defaultValue)
	{
	}

	ProgramUniformDefinition::ProgramUniformDefinition(std::string name, const glm::vec4& defaultValue, uint16_t num) noexcept
		: _name(std::move(name))
		, _type(bgfx::UniformType::Vec4)
		, _num(num)
		, _defaultValue(Data::copy(glm::value_ptr(defaultValue)))
	{
	}

	ProgramUniformDefinition::ProgramUniformDefinition(std::string name, const glm::mat3& defaultValue, uint16_t num) noexcept
		: _name(std::move(name))
		, _type(bgfx::UniformType::Mat3)
		, _num(num)
		, _defaultValue(Data::copy(glm::value_ptr(defaultValue)))
	{
	}

	ProgramUniformDefinition::ProgramUniformDefinition(std::string name, const glm::mat4& defaultValue, uint16_t num) noexcept
		: _name(std::move(name))
		, _type(bgfx::UniformType::Mat4)
		, _num(num)
		, _defaultValue(Data::copy(glm::value_ptr(defaultValue)))
	{
	}

	bool ProgramUniformDefinition::operator==(const ProgramUniformDefinition& other) const noexcept
	{
		return _name == other._name && _type == other._type && _num == other._num && 
			_defaultValue == other._defaultValue && _defaultTexture == other._defaultTexture;
	}

	bool ProgramUniformDefinition::operator!=(const ProgramUniformDefinition& other) const noexcept
	{
		return !operator==(other);
	}

	const std::string& ProgramUniformDefinition::getName() const
	{
		return _name;
	}

	bgfx::UniformType::Enum ProgramUniformDefinition::getType() const
	{
		return _type;
	}

	const Data& ProgramUniformDefinition::getDefaultValue() const
	{
		return _defaultValue;
	}

	const std::shared_ptr<Texture>& ProgramUniformDefinition::getDefaultTexture() const
	{
		return _defaultTexture;
	}

	uint16_t ProgramUniformDefinition::getNum() const
	{
		return _num;
	}

	bgfx::UniformHandle ProgramUniformDefinition::createHandle() const noexcept
	{
		return bgfx::createUniform(_name.c_str(), _type, _num);
	}

	static bgfx::VertexLayout createVertexLayout(const ProgramAttribMap& attribs) noexcept
	{
		bgfx::VertexLayout layout;
		layout.begin();
		for (const auto& pair : attribs)
		{
			layout.add(pair.first, pair.second.num, pair.second.type, pair.second.normalize);
		}
		layout.end();
		return layout;
	}

	template<typename K, typename V>
	static std::unordered_map<K, V> combineMaps(const std::unordered_map<K, V>& a, const std::unordered_map<K, V>& b) noexcept
	{
		auto combined = a;
		combined.insert(b.begin(), b.end());
		return combined;
	}

	bool ProgramBufferDefinition::operator==(const ProgramBufferDefinition& other) const noexcept
	{
		return stage == other.stage && attribs == other.attribs;
	}

	bool ProgramBufferDefinition::operator!=(const ProgramBufferDefinition& other) const noexcept
	{
		return !operator==(other);
	}

	ProgramBufferDefinition& ProgramBufferDefinition::operator+=(const ProgramBufferDefinition& other) noexcept
	{
		attribs.insert(other.attribs.begin(), other.attribs.end());
		return *this;
	}

	ProgramBufferDefinition ProgramBufferDefinition::operator+(const ProgramBufferDefinition& other) const  noexcept
	{
		return { stage, combineMaps(attribs, other.attribs) };
	}

	bool ProgramBufferDefinition::contains(const ProgramBufferDefinition& other) const noexcept
	{
		for (auto& elm : other.attribs)
		{
			if (!hasAttrib(elm.first, elm.second))
			{
				return false;
			}
		}
		return true;
	}

	bool ProgramBufferDefinition::hasAttrib(bgfx::Attrib::Enum attrib, const ProgramAttribMap& defs) const noexcept
	{
		auto itr = defs.find(attrib);
		if (itr == defs.end())
		{
			return false;
		}
		return hasAttrib(attrib, itr->second);
	}

	bool ProgramBufferDefinition::hasAttrib(bgfx::Attrib::Enum attrib, const ProgramAttribDefinition& def) const noexcept
	{
		auto itr = attribs.find(attrib);
		if (itr == attribs.end())
		{
			return false;
		}
		return itr->second == def;
	}

	bgfx::VertexLayout ProgramBufferDefinition::createVertexLayout() const noexcept
	{
		return darmok::createVertexLayout(attribs);
	}

	bgfx::VertexLayout ProgramDefinition::createVertexLayout() const noexcept
	{
		return darmok::createVertexLayout(attribs);
	}

	bool ProgramDefinition::operator==(const ProgramDefinition& other) const noexcept
	{
		return attribs == other.attribs && uniforms == other.uniforms && buffers == other.buffers;
	}

	bool ProgramDefinition::operator!=(const ProgramDefinition& other) const noexcept
	{
		return !operator==(other);
	}

	ProgramDefinition& ProgramDefinition::operator+=(const ProgramDefinition& other) noexcept
	{
		attribs.insert(other.attribs.begin(), other.attribs.end());
		uniforms.insert(other.uniforms.begin(), other.uniforms.end());
		buffers.insert(other.buffers.begin(), other.buffers.end());
		return *this;
	}

	ProgramDefinition ProgramDefinition::operator+(const ProgramDefinition& other) const noexcept
	{
		return {
			combineMaps(attribs, other.attribs),
			combineMaps(uniforms, other.uniforms),
			combineMaps(buffers, other.buffers),
		};
	}

	bool ProgramDefinition::contains(const ProgramDefinition& other) const noexcept
	{
		for (auto& elm : other.attribs)
		{
			if (!hasAttrib(elm.first, elm.second))
			{
				return false;
			}
		}
		for (auto& elm : other.uniforms)
		{
			if (!hasUniform(elm.first, elm.second))
			{
				return false;
			}
		}
		for (auto& elm : other.buffers)
		{
			if (!hasBuffer(elm.first, elm.second))
			{
				return false;
			}
		}
		return true;
	}

	bool ProgramDefinition::hasAttrib(bgfx::Attrib::Enum attrib, const ProgramAttribMap& defs) const noexcept
	{
		auto itr = defs.find(attrib);
		if (itr == defs.end())
		{
			return false;
		}
		return hasAttrib(attrib, itr->second);
	}

	bool ProgramDefinition::hasAttrib(bgfx::Attrib::Enum attrib, const ProgramAttribDefinition& def) const noexcept
	{
		auto itr = attribs.find(attrib);
		if (itr == attribs.end())
		{
			return false;
		}
		return itr->second == def;
	}

	bool ProgramDefinition::hasBuffer(ProgramBuffer buffer, const ProgramBufferMap& defs) const noexcept
	{
		auto itr = defs.find(buffer);
		if (itr == defs.end())
		{
			return false;
		}
		return hasBuffer(buffer, itr->second);
	}

	bool ProgramDefinition::hasBuffer(ProgramBuffer buffer, const ProgramBufferDefinition& def) const noexcept
	{
		auto itr = buffers.find(buffer);
		if (itr != buffers.end())
		{
			return itr->second == def;
		}
		return false;
	}

	bool ProgramDefinition::hasUniform(ProgramUniform uniform, const ProgramUniformMap& defs) const noexcept
	{
		auto itr = defs.find(uniform);
		if (itr == defs.end())
		{
			return false;
		}
		return hasUniform(uniform, itr->second);
	}


	bool ProgramDefinition::hasUniform(ProgramUniform uniform, const ProgramUniformDefinition& def) const noexcept
	{
		auto itr = uniforms.find(uniform);
		if (itr != uniforms.end())
		{
			return itr->second == def;
		}
		return false;
	}

	OptionalRef<const ProgramUniformDefinition> ProgramDefinition::getUniform(ProgramUniform uniform) const noexcept
	{
		auto itr = uniforms.find(uniform);
		if (itr != uniforms.end())
		{
			return itr->second;
		}
		return nullptr;
	}

	OptionalRef<const ProgramUniformDefinition> ProgramDefinition::getUniform(ProgramUniform uniform, std::string_view name) const noexcept
	{
		auto itr = uniforms.find(uniform);
		if (itr != uniforms.end())
		{
			auto& u = itr->second;
			if (u.getName() == name)
			{
				return u;
			}
		}
		return nullptr;
	}

	OptionalRef<const ProgramUniformDefinition> ProgramDefinition::getUniform(ProgramUniform uniform, std::string_view name, bgfx::UniformType::Enum type, uint16_t num) const noexcept
	{
		auto itr = uniforms.find(uniform);
		if (itr != uniforms.end())
		{
			auto& u = itr->second;
			if (u.getName() == name && u.getType() == type && u.getNum() == num)
			{
				return u;
			}
		}
		return nullptr;
	}
}