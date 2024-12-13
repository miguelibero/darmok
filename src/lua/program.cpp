#include "program.hpp"
#include <darmok/program.hpp>

namespace darmok
{
	void LuaProgram::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<Program>("Program", 
			sol::factories(
				[](const ProgramDefinition& def) { return std::make_shared<Program>(def); }
			),
			"vertex_layout", sol::property([](const Program& prog) { return prog.getVertexLayout().getBgfx(); })
		);
		lua.new_enum<StandardProgramType>("StandardProgramType", {
			{ "Unlit",			StandardProgramType::Unlit },
			{ "Gui",			StandardProgramType::Gui },
			{ "Forward",		StandardProgramType::Forward },
			{ "ForwardBasic",	StandardProgramType::ForwardBasic }
		});
		lua.new_usertype<StandardProgramLoader>("StandardProgramLoader", sol::no_constructor,
			"readType", &StandardProgramLoader::readType,
			"load", &StandardProgramLoader::load
		);
	}

}