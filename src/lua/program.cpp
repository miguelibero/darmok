#include "program.hpp"
#include "utils.hpp"
#include <darmok/program.hpp>

namespace darmok
{
	void LuaProgram::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<Program>("Program", 
			sol::factories(
				[](const Program::Definition& def) { return std::make_shared<Program>(def); }
			),
			"vertex_layout", sol::property([](const Program& prog) { return prog.getVertexLayout(); })
		);
		LuaUtils::newEnum<Program::Standard::Type>(lua, "StandardProgramType");
		lua.new_usertype<StandardProgramLoader>("StandardProgramLoader", sol::no_constructor,
			"load", &StandardProgramLoader::load
		);
	}

}