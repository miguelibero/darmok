#pragma once

#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <memory>
#include <string>
#include <nlohmann/json.hpp>
#include <darmok/program_fwd.hpp>

namespace bgfx
{
	struct EmbeddedShader;
}

namespace darmok
{
	class Program final
	{
	public:
		DLLEXPORT Program(const std::string& name, const bgfx::EmbeddedShader* embeddedShaders, std::string_view layoutJson);
		DLLEXPORT Program(const bgfx::ProgramHandle& handle, const bgfx::VertexLayout& layout) noexcept;
		DLLEXPORT Program(const Program& other) = delete;
		DLLEXPORT Program& operator=(const Program& other) = delete;
		DLLEXPORT ~Program() noexcept;

		[[nodiscard]] DLLEXPORT const bgfx::ProgramHandle& getHandle() const noexcept;
		[[nodiscard]] DLLEXPORT const bgfx::VertexLayout& getVertexLayout() const noexcept;

		DLLEXPORT static void readVertexLayoutJson(const nlohmann::json& json, bgfx::VertexLayout& layout) noexcept;

	private:
		bgfx::ProgramHandle _handle;
		bgfx::VertexLayout _layout;

		static bgfx::Attrib::Enum getBgfxAttrib(const std::string_view name) noexcept;
		static bgfx::AttribType::Enum getBgfxAttribType(const std::string_view name) noexcept;
	};

	class BX_NO_VTABLE IProgramLoader
	{
	public:
		using result_type = std::shared_ptr<Program>;

		DLLEXPORT virtual ~IProgramLoader() = default;
		DLLEXPORT virtual result_type operator()(std::string_view name) = 0;
	};

	class StandardProgramLoaderImpl;

	class StandardProgramLoader final
	{
	public:
		using result_type = std::shared_ptr<Program>;

		DLLEXPORT StandardProgramLoader() noexcept;
		DLLEXPORT ~StandardProgramLoader() noexcept;
		DLLEXPORT virtual result_type operator()(StandardProgramType type) noexcept;
	private:
		std::unique_ptr<StandardProgramLoaderImpl> _impl;
	};
}