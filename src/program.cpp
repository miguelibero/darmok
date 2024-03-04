#include "program.hpp"
#include <darmok/data.hpp>
#include <unordered_map>

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

	DataProgramLoader::DataProgramLoader(IDataLoader& dataLoader)
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

	bgfx::ShaderHandle DataProgramLoader::loadShader(const std::string& name)
	{
		std::string dataName = name + getShaderExt() + ".bin";
		auto data = _dataLoader(dataName);
		if (data == nullptr || data->empty())
		{
			throw std::runtime_error("got empty data");
		}
		bgfx::ShaderHandle handle = bgfx::createShader(data->makeRef());
		bgfx::setName(handle, name.c_str());
		return handle;
	}

	std::shared_ptr<Program> DataProgramLoader::operator()(std::string_view vertexName, std::string_view fragmentName)
	{
		bgfx::ShaderHandle vsh = loadShader(std::string(fragmentName));
		bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
		if (!fragmentName.empty())
		{
			fsh = loadShader(std::string(fragmentName));
		}
		auto handle = bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
		return std::make_shared<Program>(handle);
	}

	bgfx::UniformHandle ProgramUniformDefinition::createHandle() const noexcept
	{
		return bgfx::createUniform(name.c_str(), type, num);
	}

	bgfx::VertexLayout ProgramDefinition::createVertexLayout() const noexcept
	{
		bgfx::VertexLayout layout;
		layout.begin();
		for (auto& pair : attribs)
		{
			layout.add(pair.first, pair.second.num, pair.second.type, pair.second.normalize);
		}
		layout.end();
		return layout;
	}
}