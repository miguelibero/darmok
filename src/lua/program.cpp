#include "program.hpp"
#include <darmok/program.hpp>

namespace darmok
{
	void LuaProgram::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<Program>("Program", sol::no_constructor,
			"vertex_layout", sol::property(&Program::getVertexLayout)
		);
	}

}