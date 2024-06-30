#include "program_standard.hpp"
#include "embedded_shader.hpp"
#include "generated/shaders/gui.vertex.h"
#include "generated/shaders/gui.fragment.h"
#include "generated/shaders/gui.vlayout.h"
#include "generated/shaders/unlit.vertex.h"
#include "generated/shaders/unlit.fragment.h"
#include "generated/shaders/unlit.vlayout.h"
#include "generated/shaders/debug.vertex.h"
#include "generated/shaders/debug.fragment.h"
#include "generated/shaders/debug.vlayout.h"
#include "generated/shaders/forward_phong.vertex.h"
#include "generated/shaders/forward_phong.fragment.h"
#include "generated/shaders/forward_phong.vlayout.h"
#include "generated/shaders/forward_pbr.vertex.h"
#include "generated/shaders/forward_pbr.fragment.h"
#include "generated/shaders/forward_pbr.vlayout.h"
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/program.hpp>
#include <darmok/vertex_layout.hpp>
#include <stdexcept>

namespace darmok
{
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

	std::optional<StandardProgramType> StandardProgramLoader::getType(std::string_view name) noexcept
	{
		return StandardProgramLoaderImpl::getType(name);
	}

	bgfx::VertexLayout StandardProgramLoader::getVertexLayout(StandardProgramType type) noexcept
	{
		return StandardProgramLoaderImpl::getVertexLayout(type);
	}

	static bgfx::VertexLayout getVertexLayout(StandardProgramType type) noexcept
	{
		return StandardProgramLoader::getVertexLayout(type);
	}

	const bgfx::EmbeddedShader StandardProgramLoaderImpl::_embeddedShaders[] =
	{
		BGFX_EMBEDDED_SHADER(gui_vertex),
		BGFX_EMBEDDED_SHADER(gui_fragment),
		BGFX_EMBEDDED_SHADER(unlit_vertex),
		BGFX_EMBEDDED_SHADER(unlit_fragment),
		BGFX_EMBEDDED_SHADER(debug_vertex),
		BGFX_EMBEDDED_SHADER(debug_fragment),
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
		{StandardProgramType::Debug, "debug"},
		{StandardProgramType::ForwardPhong, "forward_phong"},
		{StandardProgramType::ForwardPhysical, "forward_pbr"},
	};

	const std::unordered_map<StandardProgramType, DataView> StandardProgramLoaderImpl::_embeddedShaderVertexLayouts
	{
		{StandardProgramType::Gui, DataView::fromArray(gui_vlayout) },
		{StandardProgramType::Unlit, DataView::fromArray(unlit_vlayout) },
		{StandardProgramType::Debug, DataView::fromArray(debug_vlayout) },
		{StandardProgramType::ForwardPhong, DataView::fromArray(forward_phong_vlayout) },
		{StandardProgramType::ForwardPhysical, DataView::fromArray(forward_pbr_vlayout) },
	};

	std::optional<StandardProgramType> StandardProgramLoaderImpl::getType(std::string_view name) noexcept
	{
		if (name == "Gui")
		{
			return StandardProgramType::Gui;
		}
		if (name == "Unlit")
		{
			return StandardProgramType::Unlit;
		}
		if (name == "ForwardPhong")
		{
			return StandardProgramType::ForwardPhong;
		}
		if (name == "ForwardPhysical")
		{
			return StandardProgramType::ForwardPhysical;
		}
		return std::nullopt;
	}

	bgfx::VertexLayout StandardProgramLoaderImpl::getVertexLayout(StandardProgramType type) noexcept
	{
		auto itr = _embeddedShaderVertexLayouts.find(type);
		bgfx::VertexLayout layout;
		if (itr != _embeddedShaderVertexLayouts.end())
		{
			DataInputStream::read(itr->second, layout);
		}
		return layout;
	}

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
		}
		auto layout = getVertexLayout(type);
		return std::make_shared<Program>(itr->second, _embeddedShaders, itr2->second);
	}
}