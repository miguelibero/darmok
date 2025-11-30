#include "lua/program.hpp"
#include "lua/utils.hpp"
#include "lua/protobuf.hpp"
#include "lua/scene_serialize.hpp"
#include <darmok/program.hpp>

namespace darmok
{
	void LuaProgram::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<Program>("Program", 
			sol::factories(
				[](const Program::Definition& def)
				{
					auto result = Program::load(def);
					auto prog = LuaUtils::unwrapExpected(std::move(result));
					return std::make_shared<Program>(std::move(prog));
				}
			),
			"vertex_layout", sol::property([](const Program& prog) { return prog.getVertexLayout(); })
		);
		LuaUtils::newEnum<Program::Standard::Type>(lua, "StandardProgramType");
		lua.new_usertype<StandardProgramLoader>("StandardProgramLoader", sol::no_constructor,
			"load", &StandardProgramLoader::load
		);

		auto progSrc = LuaUtils::newProtobuf<Program::Source>(lua, "ProgramSource")
			.protobufProperty<protobuf::Varying>("varying");
		progSrc.userType["get_scene_asset"] = [](LuaSceneDefinition& scene, std::string_view path)
			{
				return scene.getAsset<Program::Source>(path);
			};

		auto progDef = LuaUtils::newProtobuf<Program::Definition>(lua, "ProgramDefinition")
			.protobufProperty<protobuf::ProgramProfile>("profiles")
			.protobufProperty<protobuf::Varying>("varying");
		progDef.userType["get_scene_asset"] = [](LuaSceneDefinition& scene, std::string_view path)
			{
				return scene.getAsset<Program::Definition>(path);
			};
	}

}