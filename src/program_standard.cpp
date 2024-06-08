#include "program_standard.hpp"
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
#include <darmok/data_stream.hpp>
#include <darmok/program.hpp>
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

	const std::unordered_map<StandardProgramType, DataView> StandardProgramLoaderImpl::_embeddedShaderVertexLayouts
	{
		{StandardProgramType::Gui, DataView::fromArray(gui_vlayout) },
		{StandardProgramType::Unlit, DataView::fromArray(unlit_vlayout) },
		{StandardProgramType::ForwardPhong, DataView::fromArray(forward_phong_vlayout) },
		{StandardProgramType::ForwardPhysical, DataView::fromArray(forward_pbr_vlayout) },
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