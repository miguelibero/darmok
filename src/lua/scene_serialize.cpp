#include "lua/scene_serialize.hpp"
#include "lua/scene.hpp"
#include <darmok/scene_serialize.hpp>
#include <darmok/mesh.hpp>
#include <darmok/asset_pack.hpp>

namespace darmok
{
	LuaSceneConverter::LuaSceneConverter()
		: _converter{ std::make_unique<SceneConverter>() }
	{
	}

	LuaSceneConverter::LuaSceneConverter(AssetContext& assets)
		: LuaSceneConverter()
	{
		_converter->setAssetPackConfig({
			.fallback = assets
		});
	}

	LuaSceneConverter::~LuaSceneConverter() = default;

	void LuaSceneConverter::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaSceneConverter>("SceneConverter", sol::constructors<
			LuaSceneConverter(), LuaSceneConverter(AssetContext&)>(),
			"parent", sol::property(&LuaSceneConverter::setParent),
			"renderable_setup", sol::property(&LuaSceneConverter::setRenderableSetup),
			sol::meta_function::call, &LuaSceneConverter::run
		);
	}

	void LuaSceneConverter::setParent(const LuaEntity& entity)
	{
		_converter->setParent(entity.getReal());
	}

	void LuaSceneConverter::setRenderableSetup(const sol::function& func)
	{
		_converter->addComponentListener<Renderable>([this, func](const Renderable::Definition& def, Entity entity)
		{
			func(def, LuaEntity{ entity, _scene });
		});
	}

	void LuaSceneConverter::run(const protobuf::Scene& sceneDef, std::shared_ptr<Scene> scene)
	{
		_scene = scene;
		auto result = (*_converter)(sceneDef, *scene);
		if (!result)
		{
			throw std::runtime_error{ result.error() };
		}
	}
}