#pragma once

#include "lua/lua.hpp"

namespace darmok
{
	class Scene;
	class SceneLoader;
	class LuaEntity;
	class AssetContext;
	class AssetPack;

	namespace protobuf
	{
		class Scene;
	};

	class LuaSceneLoader final
	{
	public:
		LuaSceneLoader();
		LuaSceneLoader(AssetContext& assets);
		~LuaSceneLoader();

		static void bind(sol::state_view& lua) noexcept;
	private:
		LuaEntity run(const protobuf::Scene& sceneDef, std::shared_ptr<Scene> scene);
		void setParent(const LuaEntity& entity);
		void setRenderableSetup(const sol::function& func);
		AssetPack& getAssetPack();

		std::unique_ptr<SceneLoader> _loader;
		std::weak_ptr<Scene> _scene;
	};
}