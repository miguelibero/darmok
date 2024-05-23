#include "program.hpp"
#include <darmok/program.hpp>

namespace darmok
{
    LuaProgram::LuaProgram(const std::shared_ptr<Program>& program) noexcept
		: _program(program)
	{
	}

	const bgfx::VertexLayout& LuaProgram::getVertexLayout() const noexcept
	{
		return _program->getVertexLayout();
	}

	std::shared_ptr<Program> LuaProgram::getReal() const noexcept
	{
		return _program;
	}

	void LuaProgram::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaProgram>("Program",
			sol::constructors<>(),
			"vertex_layout", sol::property(&LuaProgram::getVertexLayout)
		);

		lua.new_enum<StandardProgramType>("StandardProgramType", {
			{ "gui", StandardProgramType::Gui },
			{ "unlit", StandardProgramType::Unlit },
			{ "forward_phong", StandardProgramType::ForwardPhong },
			{ "forward_pbr", StandardProgramType::ForwardPbr }
		});
	}

}