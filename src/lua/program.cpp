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
			"load", [](StandardProgramLoader::Type type)
			{
				return LuaUtils::unwrapExpected(StandardProgramLoader::load(type));
			},
			"load_definition", [](StandardProgramLoader::Type type)
			{
				return LuaUtils::unwrapExpected(StandardProgramLoader::loadDefinition(type));
			}
		);

		{
			LuaProtobufBinding<protobuf::VertexAttribute>{lua, "VertexAttribute"};
			LuaProtobufBinding<protobuf::FragmentAttribute>{lua, "FragmentAttribute"};

			LuaProtobufBinding<protobuf::VertexLayout>{lua, "VertexLayout"}
				.protobufProperty<protobuf::VertexAttribute>("attributes");

			LuaProtobufBinding<protobuf::FragmentLayout>{lua, "FragmentLayout"}
				.protobufProperty<protobuf::FragmentAttribute>("attributes");

			LuaProtobufBinding<protobuf::Varying>{lua, "Varying"}
				.protobufProperty<protobuf::VertexLayout>("vertex")
				.protobufProperty<protobuf::FragmentLayout>("fragment");
		}

		auto src = lua.new_usertype<Program::Source>("ProgramSource",
			sol::factories(
				[]() { return Program::createSource(); }
			),
			"get_scene_asset", [](LuaSceneDefinition& scene, std::string_view path)
			{
				return scene.getAsset<Program::Source>(path);
			}
		);
		LuaProtobufBinding{ std::move(src) }
			.protobufProperty<protobuf::Varying>("varying");

		LuaProtobufBinding<protobuf::Shader>{lua, "ShaderDefinition"};
		LuaProtobufBinding<protobuf::ProgramProfile>{lua, "ProgramProfileDefinition"};

		auto def = lua.new_usertype<Program::Definition>("ProgramDefinition",
			sol::factories(
				[]() { return Program::createDefinition(); }
			),
			"get_scene_asset", [](LuaSceneDefinition& scene, std::string_view path)
			{
				return scene.getAsset<Program::Definition>(path);
			}
		);
		LuaProtobufBinding{ std::move(def) }
			.protobufProperty<protobuf::ProgramProfile>("profiles")
			.protobufProperty<protobuf::Varying>("varying");
	}

}