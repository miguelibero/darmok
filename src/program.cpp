#include "program.hpp"
#include <unordered_map>
#include <optional>

#include <darmok/data.hpp>
#include <darmok/vertex.hpp>
#include <darmok/vertex_layout.hpp>

#include "embedded_shader.hpp"
#include "generated/shaders/gui.vertex.h"
#include "generated/shaders/gui.fragment.h"
#include "generated/shaders/gui.vlayout.h"
#include "generated/shaders/unlit.vertex.h"
#include "generated/shaders/unlit.fragment.h"
#include "generated/shaders/unlit.vlayout.h"
#include "generated/shaders/forward_phong.vertex.h"
#include "generated/shaders/forward_phong.fragment.h"
#include "generated/shaders/forward_phong.vlayout.h"
#include "generated/shaders/forward_pbr.vertex.h"
#include "generated/shaders/forward_pbr.fragment.h"
#include "generated/shaders/forward_pbr.vlayout.h"

namespace darmok
{
	Program::Program(const std::string& name, const bgfx::EmbeddedShader* embeddedShaders, std::string_view layoutJson)
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
		_handle = bgfx::createProgram(vertHandle, fragHandle, true);
		auto json = nlohmann::ordered_json::parse(layoutJson);
		VertexLayoutUtils::readJson(json, _layout);
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

	const DataProgramLoader::Suffixes DataProgramLoader::defaultSuffixes = Suffixes{ "_vertex", "_fragment", "_vertex_layout" };

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
		auto layout = _vertexLayoutLoader(nameStr + _suffixes.vertexLayout);
		return std::make_shared<Program>(handle, layout);
	}

	StandardProgramLoader::StandardProgramLoader() noexcept
		: _impl(std::make_unique<StandardProgramLoaderImpl>())
	{
	}

	StandardProgramLoader::~StandardProgramLoader() noexcept
	{
	}

	StandardProgramLoader::result_type StandardProgramLoader::operator()(StandardProgramType type) noexcept
	{
		return (*_impl)(type);
	}

	const bgfx::EmbeddedShader StandardProgramLoaderImpl::_embeddedShaders[] =
	{
		BGFX_EMBEDDED_SHADER(gui_vertex),
		BGFX_EMBEDDED_SHADER(gui_fragment),
		BGFX_EMBEDDED_SHADER(unlit_vertex),
		BGFX_EMBEDDED_SHADER(unlit_fragment),
		BGFX_EMBEDDED_SHADER(forward_phong_vertex),
		BGFX_EMBEDDED_SHADER(forward_phong_fragment),
		BGFX_EMBEDDED_SHADER(forward_pbr_vertex),
		BGFX_EMBEDDED_SHADER(forward_pbr_fragment),
		BGFX_EMBEDDED_SHADER_END()
	};

	const std::unordered_map<StandardProgramType, std::string> StandardProgramLoaderImpl::_embeddedShaderNames
	{
		{StandardProgramType::Gui, "gui"},
		{StandardProgramType::Unlit, "unlit"},
		{StandardProgramType::ForwardPhong, "forward_phong"},
		{StandardProgramType::ForwardPhysical, "forward_pbr"},
	};

	const std::unordered_map<StandardProgramType, std::string_view> StandardProgramLoaderImpl::_embeddedShaderVertexLayouts
	{
		{StandardProgramType::Gui, gui_vlayout},
		{StandardProgramType::Unlit, unlit_vlayout},
		{StandardProgramType::ForwardPhong, forward_phong_vlayout},
		{StandardProgramType::ForwardPhysical, forward_pbr_vlayout},
	};

	std::shared_ptr<Program> StandardProgramLoaderImpl::operator()(StandardProgramType type) const noexcept
	{
		auto itr = _embeddedShaderNames.find(type);
		if (itr == _embeddedShaderNames.end())
		{
			return nullptr;
		}
		auto itr2 = _embeddedShaderVertexLayouts.find(type);
		if (itr2 == _embeddedShaderVertexLayouts.end())
		{
			return nullptr;
		};
		return std::make_shared<Program>(itr->second, _embeddedShaders, itr2->second);
	}
}