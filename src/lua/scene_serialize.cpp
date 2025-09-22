#include "lua/scene_serialize.hpp"
#include "lua/scene.hpp"
#include <darmok/scene_serialize.hpp>
#include <darmok/mesh.hpp>
#include <darmok/asset_pack.hpp>

namespace darmok
{
	void LuaSceneConverter::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<SceneConverter>("SceneConverter", sol::factories(
				[]() { return std::make_shared<SceneConverter>(); },
				[](AssetContext& assets) { 
					auto converter = std::make_shared<SceneConverter>();
					converter->setAssetPackConfig({
						.fallback = assets
					});
					return converter;
				}
			),
			"parent", sol::property(&LuaSceneConverter::setParent),
			"mesh_setup", sol::property(&LuaSceneConverter::setMeshSetup),
			sol::meta_function::call, &LuaSceneConverter::run
		);
	}

	void LuaSceneConverter::setParent(SceneConverter& converter, const LuaEntity& entity)
	{
		converter.setParent(entity.getReal());
	}

	void LuaSceneConverter::setMeshSetup(SceneConverter& converter, const sol::function& func)
	{
		converter.addComponentListener<Mesh>([func](const Mesh::Definition& def, Entity entity)
		{
			func(def, entity);
		});
	}

	void LuaSceneConverter::run(SceneConverter& converter, const protobuf::Scene& sceneDef, Scene& scene)
	{
		auto result = converter(sceneDef, scene);
		if (!result)
		{
			throw std::runtime_error{ result.error() };
		}
	}
}