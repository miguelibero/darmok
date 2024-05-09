#pragma once

#include <memory>
#include <bgfx/bgfx.h>
#include "sol.hpp"

namespace darmok
{
    class Program;

    class LuaProgram final
	{
	public:
		LuaProgram(const std::shared_ptr<Program>& program) noexcept;
		const bgfx::VertexLayout& getVertexLayout() const noexcept;
		std::shared_ptr<Program> getReal() const noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<Program> _program;
	};
}