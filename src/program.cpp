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

	ProgramUniformDefinition::ProgramUniformDefinition(std::string name, bgfx::UniformType::Enum type, uint16_t num) noexcept
		: _name(std::move(name))
		, _type(type)
		, _num(num)
	{
	}

	ProgramUniformDefinition::ProgramUniformDefinition(std::string name, std::shared_ptr<Texture>& defaultValue, uint16_t num) noexcept
		: _name(std::move(name))
		, _type(bgfx::UniformType::Sampler)
		, _defaultTexture(defaultValue)
		, _num(num)
	{
	}

	ProgramUniformDefinition::ProgramUniformDefinition(std::string name, const glm::vec4& defaultValue, uint16_t num) noexcept
		: _name(std::move(name))
		, _type(bgfx::UniformType::Vec4)
		, _defaultValue(Data::copy(glm::value_ptr(defaultValue)))
		, _num(num)
	{
	}

	ProgramUniformDefinition::ProgramUniformDefinition(std::string name, const glm::mat3& defaultValue, uint16_t num) noexcept
		: _name(std::move(name))
		, _type(bgfx::UniformType::Mat3)
		, _defaultValue(Data::copy(glm::value_ptr(defaultValue)))
		, _num(num)
	{
	}

	ProgramUniformDefinition::ProgramUniformDefinition(std::string name, const glm::mat4& defaultValue, uint16_t num) noexcept
		: _name(std::move(name))
		, _type(bgfx::UniformType::Mat4)
		, _defaultValue(Data::copy(glm::value_ptr(defaultValue)))
		, _num(num)
	{
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

	bgfx::VertexLayout ProgramDefinition::createVertexLayout() const noexcept
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
}