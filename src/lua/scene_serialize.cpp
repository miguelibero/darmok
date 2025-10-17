#include "lua/scene_serialize.hpp"
#include "lua/scene.hpp"
#include <darmok/scene_serialize.hpp>
#include <darmok/mesh.hpp>
#include <darmok/asset_pack.hpp>
#include <darmok/stream.hpp>

namespace darmok
{
	LuaSceneLoader::LuaSceneLoader()
		: _loader{ std::make_unique<SceneLoader>() }
	{
	}

	LuaSceneLoader::LuaSceneLoader(AssetContext& assets)
		: LuaSceneLoader()
	{
		_loader->setAssetPackConfig({
			.fallback = assets
		});
	}

	LuaSceneLoader::~LuaSceneLoader() = default;

	void LuaSceneLoader::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaSceneLoader>("SceneLoader", sol::constructors<
			LuaSceneLoader(), LuaSceneLoader(AssetContext&)>(),
			"parent", sol::property(&LuaSceneLoader::setParent),
			"renderable_setup", sol::property(&LuaSceneLoader::setRenderableSetup),
			"asset_pack", sol::property(&LuaSceneLoader::getAssetPack),
			sol::meta_function::call, &LuaSceneLoader::run
		);
	}

	AssetPack& LuaSceneLoader::getAssetPack()
	{
		return _loader->getAssetPack();
	}

	void LuaSceneLoader::setParent(const LuaEntity& entity)
	{
		_loader->setParent(entity.getReal());
	}

	void LuaSceneLoader::setRenderableSetup(const sol::function& func)
	{
		_loader->addComponentListener<Renderable>([this, func](const Renderable::Definition& def, Entity entity)
		{
			auto result = func(def, LuaEntity{ entity, _scene });
			LuaUtils::throwResult(result, "renderable setup callback");
		});
	}

	LuaEntity LuaSceneLoader::run(const protobuf::Scene& sceneDef, std::shared_ptr<Scene> scene)
	{
		_scene = scene;
		auto result = (*_loader)(sceneDef, *scene);
		if (!result)
		{
			throw sol::error{ result.error() };
		}
		return { result.value(), scene };
	}
}