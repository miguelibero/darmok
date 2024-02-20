#include "program.hpp"
#include <darmok/data.hpp>
#include <unordered_map>

#include <bgfx/embedded_shader.h>
#include "generated/shaders/debug_vertex.h"
#include "generated/shaders/debug_fragment.h"
#include "generated/shaders/sprite_vertex.h"
#include "generated/shaders/sprite_fragment.h"
#include "generated/shaders/basic_vertex.h"
#include "generated/shaders/basic_fragment.h"

namespace darmok
{
    Program::Program(const bgfx::ProgramHandle& handle)
		: _handle(handle)
	{
	}

    Program::~Program()
    {
    }

	const bgfx::ProgramHandle& Program::getHandle() const
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
		case bgfx::RendererType::Direct3D11:
		case bgfx::RendererType::Direct3D12: return "dx11";  break;
		case bgfx::RendererType::Agc:
		case bgfx::RendererType::Gnm:        return "pssl";  break;
		case bgfx::RendererType::Metal:      return "metal"; break;
		case bgfx::RendererType::Nvn:		 return "nvn";   break;
		case bgfx::RendererType::OpenGL:     return "glsl";  break;
		case bgfx::RendererType::OpenGLES:   return "essl";  break;
		case bgfx::RendererType::Vulkan:     return "spv"; break;

		case bgfx::RendererType::Count:
			BX_ASSERT(false, "You should not be here!");
			break;
		}
		return "???";
	}

	bgfx::ShaderHandle DataProgramLoader::loadShader(const std::string& name)
	{
		std::string dataName = name + "." + getShaderExt() + ".bin";
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

	static const bgfx::EmbeddedShader _embeddedShaders[] =
	{
		BGFX_EMBEDDED_SHADER(debug_vertex),
		BGFX_EMBEDDED_SHADER(debug_fragment),
		BGFX_EMBEDDED_SHADER(sprite_vertex),
		BGFX_EMBEDDED_SHADER(sprite_fragment),
		BGFX_EMBEDDED_SHADER(basic_vertex),
		BGFX_EMBEDDED_SHADER(basic_fragment),
		BGFX_EMBEDDED_SHADER_END()
	};

	static const std::unordered_map<EmbeddedProgramType, std::string> _embeddedShaderNames
	{
		{EmbeddedProgramType::Basic, "basic"},
		{EmbeddedProgramType::Sprite, "sprite"},
		{EmbeddedProgramType::Debug, "debug"},
	};

	std::shared_ptr<Program> EmbeddedProgramLoader::operator()(EmbeddedProgramType type)
	{
		auto itr = _embeddedShaderNames.find(type);
		if (itr == _embeddedShaderNames.end())
		{
			return nullptr;
		}
		auto renderer = bgfx::getRendererType();
		auto handle = bgfx::createProgram(
			bgfx::createEmbeddedShader(_embeddedShaders, renderer, (itr->second + "_vertex").c_str()),
			bgfx::createEmbeddedShader(_embeddedShaders, renderer, (itr->second + "_fragment").c_str()),
			true
		);
		return std::make_shared<Program>(handle);
	}
}