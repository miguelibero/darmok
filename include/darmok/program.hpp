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
		Program(const std::string& name, const bgfx::EmbeddedShader* embeddedShaders, std::string_view layoutJson);
		Program(const bgfx::ProgramHandle& handle, const bgfx::VertexLayout& layout) noexcept;
		Program(const Program& other) = delete;
		Program& operator=(const Program& other) = delete;
		~Program() noexcept;
		[[nodiscard]] const bgfx::ProgramHandle& getHandle() const noexcept;
		const bgfx::VertexLayout& getVertexLayout() const noexcept;

		static void readVertexLayoutJson(const nlohmann::json& json, bgfx::VertexLayout& layout) noexcept;

		static bgfx::Attrib::Enum getBgfxAttrib(const std::string_view name) noexcept;
		static bgfx::AttribType::Enum getBgfxAttribType(const std::string_view name) noexcept;

	private:
		bgfx::ProgramHandle _handle;
		bgfx::VertexLayout _layout;
	};

	class BX_NO_VTABLE IProgramLoader
	{
	public:
		using result_type = std::shared_ptr<Program>;

		virtual ~IProgramLoader() = default;
		virtual result_type operator()(std::string_view name) = 0;
	};

	class StandardProgramLoaderImpl;

	class StandardProgramLoader final
	{
	public:
		StandardProgramLoader() noexcept;
		~StandardProgramLoader() noexcept;
		using result_type = std::shared_ptr<Program>;
		virtual result_type operator()(StandardProgramType type) noexcept;
	private:
		std::unique_ptr<StandardProgramLoaderImpl> _impl;
	};
}