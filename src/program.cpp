#include <darmok/program.hpp>
#include "embedded_shader.hpp"
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/vertex.hpp>
#include <darmok/vertex_layout.hpp>

namespace darmok
{
	Program::Program(const std::string& name, const bgfx::EmbeddedShader* embeddedShaders, const DataView& vertexLayout)
		: _handle{ bgfx::kInvalidHandle }
	{
		auto renderer = bgfx::getRendererType();
		auto vertName = name + "_vertex";
		auto vertHandle = bgfx::createEmbeddedShader(embeddedShaders, renderer, vertName.c_str());
		if (!isValid(vertHandle))
		{
			throw std::runtime_error("could not load embedded vertex shader" + vertName);
		}
		auto fragName = name + "_fragment";
		auto fragHandle = bgfx::createEmbeddedShader(embeddedShaders, renderer, fragName.c_str());
		if (!isValid(fragHandle))
		{
			throw std::runtime_error("could not load embedded fragment shader" + fragName);
		}
		DataInputStream::read(vertexLayout, _layout);
		_handle = bgfx::createProgram(vertHandle, fragHandle, true);
	}

    Program::Program(const bgfx::ProgramHandle& handle, const bgfx::VertexLayout& layout) noexcept
		: _handle(handle)
		, _layout(layout)
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

	const bgfx::VertexLayout& Program::getVertexLayout() const noexcept
	{
		return _layout;
	}

	const DataProgramLoader::Suffixes& DataProgramLoader::getDefaultSuffixes() noexcept
	{
		static const DataProgramLoader::Suffixes suffixes{ "_vertex", "_fragment", "_vertex_layout" };
		return suffixes;
	}

	DataProgramLoader::DataProgramLoader(IDataLoader& dataLoader, IVertexLayoutLoader& vertexLayoutLoader, Suffixes suffixes) noexcept
		: _dataLoader(dataLoader)
		, _vertexLayoutLoader(vertexLayoutLoader)
		, _suffixes(suffixes)
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
		if (data.empty())
		{
			throw std::runtime_error("got empty data");
		}
		bgfx::ShaderHandle handle = bgfx::createShader(data.makeRef());
		bgfx::setName(handle, filePath.c_str());
		return handle;
	}

	bgfx::VertexLayout DataProgramLoader::loadVertexLayout(std::string_view name)
	{
		return _vertexLayoutLoader(std::string(name) + _suffixes.vertexLayout);
	}

	std::shared_ptr<Program> DataProgramLoader::operator()(std::string_view name)
	{
		std::string nameStr(name);
		const bgfx::ShaderHandle vsh = loadShader(nameStr + _suffixes.vertex);
		bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
		if (!name.empty())
		{
			fsh = loadShader(nameStr + _suffixes.fragment);
		}
		auto handle = bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
		auto layout = loadVertexLayout(name);
		return std::make_shared<Program>(handle, layout);
	}
}