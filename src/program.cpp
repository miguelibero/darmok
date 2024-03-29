#include "program.hpp"
#include <unordered_map>
#include <darmok/data.hpp>

#include "embedded_shader.hpp"
#include "generated/shaders/unlit_vertex.h"
#include "generated/shaders/unlit_fragment.h"
#include "generated/shaders/forward_phong_vertex.h"
#include "generated/shaders/forward_phong_fragment.h"
#include "generated/shaders/sprite_vertex.h"
#include "generated/shaders/sprite_fragment.h"

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

	static const bgfx::EmbeddedShader _embeddedShaders[] =
	{
		BGFX_EMBEDDED_SHADER(unlit_vertex),
		BGFX_EMBEDDED_SHADER(unlit_fragment),
		BGFX_EMBEDDED_SHADER(forward_phong_vertex),
		BGFX_EMBEDDED_SHADER(forward_phong_fragment),
		BGFX_EMBEDDED_SHADER(sprite_vertex),
		BGFX_EMBEDDED_SHADER(sprite_fragment),
		BGFX_EMBEDDED_SHADER_END()
	};

	static const std::unordered_map<StandardProgramType, std::string> _embeddedShaderNames
	{
		{StandardProgramType::Unlit, "unlit"},
		{StandardProgramType::ForwardPhong, "forward_phong"},
		{StandardProgramType::Sprite, "sprite"},
	};

	std::shared_ptr<Program> Program::createStandard(StandardProgramType type) noexcept
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

	DataProgramLoader::DataProgramLoader(IDataLoader& dataLoader, const std::string& vertexSuffix, const std::string& fragmentSuffix) noexcept
		: _dataLoader(dataLoader)
		, _vertexSuffix(vertexSuffix)
		, _fragmentSuffix(fragmentSuffix)
	{
	}

	static std::string getShaderExt()
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

	std::shared_ptr<Program> DataProgramLoader::operator()(std::string_view name)
	{
		std::string nameStr(name);
		const bgfx::ShaderHandle vsh = loadShader(nameStr + _vertexSuffix);
		bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
		if (!name.empty())
		{
			fsh = loadShader(nameStr + _fragmentSuffix);
		}
		auto handle = bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
		return std::make_shared<Program>(handle);
	}
}