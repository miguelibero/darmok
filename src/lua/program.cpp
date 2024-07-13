#include "program.hpp"
#include <darmok/program.hpp>

namespace darmok
{
	void LuaProgram::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<Program>("Program", sol::no_constructor,
			"vertex_layout", sol::property(&Program::getVertexLayout)
		);

		lua.new_usertype<ProgramGroup>("ProgramGroup", sol::no_constructor,
			"get_program", &ProgramGroup::getProgram,
			"set_program", &ProgramGroup::setProgram,
			"load_basic", &ProgramGroup::loadBasic
		);

	}

}