#pragma once

#include "lua/lua.hpp"

namespace darmok
{
	class Scene;
	class SceneConverter;
	class LuaEntity;
	class AssetContext;

	namespace protobuf
	{
		class Scene;
	};

	class LuaSceneConverter final
	{
	public:
		LuaSceneConverter();
		LuaSceneConverter(AssetContext& assets);
		~LuaSceneConverter();

		static void bind(sol::state_view& lua) noexcept;
	private:
		void run(const protobuf::Scene& sceneDef, std::shared_ptr<Scene> scene);
		void setParent(const LuaEntity& entity);
		void setRenderableSetup(const sol::function& func);

		std::unique_ptr<SceneConverter> _converter;
		std::weak_ptr<Scene> _scene;
	};
}