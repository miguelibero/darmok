#include "program.hpp"
#include <darmok/program.hpp>

namespace darmok
{
	void LuaProgram::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<Program>("Program", 
			sol::factories(
				[](StandardProgramType type) { return std::make_shared<Program>(type); }
			),
			"vertex_layout", sol::property([](const Program& prog) { return prog.getVertexLayout().getBgfx(); })
		);
		lua.new_enum<StandardProgramType>("StandardProgramType", {
			{ "Unlit",		StandardProgramType::Unlit },
			{ "Gui",		StandardProgramType::Gui },
			{ "Forward",	StandardProgramType::Forward }
		});
	}

}