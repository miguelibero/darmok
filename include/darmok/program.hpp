#pragma once

#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <memory>
#include <string>
#include <darmok/program_fwd.hpp>

namespace bgfx
{
	struct EmbeddedShader;
}

namespace darmok
{
	class DataView;

	class Program final
	{
	public:
		DLLEXPORT Program(const std::string& name, const bgfx::EmbeddedShader* embeddedShaders, const DataView& vertexLayout);
		DLLEXPORT Program(const bgfx::ProgramHandle& handle, const bgfx::VertexLayout& layout) noexcept;
		DLLEXPORT ~Program() noexcept;
		Program(const Program& other) = delete;
		Program& operator=(const Program& other) = delete;

		[[nodiscard]] DLLEXPORT const bgfx::ProgramHandle& getHandle() const noexcept;
		[[nodiscard]] DLLEXPORT const bgfx::VertexLayout& getVertexLayout() const noexcept;
	private:
		bgfx::ProgramHandle _handle;
		bgfx::VertexLayout _layout;
	};

	class BX_NO_VTABLE IProgramLoader
	{
	public:
		using result_type = std::shared_ptr<Program>;

		DLLEXPORT virtual ~IProgramLoader() = default;
		DLLEXPORT virtual result_type operator()(std::string_view name) = 0;
	};
}