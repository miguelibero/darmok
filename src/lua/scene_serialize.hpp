#pragma once

#include "lua/lua.hpp"

namespace darmok
{
	class Scene;
	class SceneConverter;
	class LuaEntity;

	namespace protobuf
	{
		class Scene;
	};

	class LuaSceneConverter final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:
		static void run(SceneConverter& converter, const protobuf::Scene& sceneDef, Scene& scene);
		static void setParent(SceneConverter& converter, const LuaEntity& entity);
		static void setMeshSetup(SceneConverter& converter, const sol::function& func);
	};
}