#include "lua/scene_serialize.hpp"
#include "lua/scene.hpp"
#include "lua/protobuf.hpp"

#include <darmok/scene_serialize.hpp>
#include <darmok/transform.hpp>
#include <darmok/mesh.hpp>
#include <darmok/asset_pack.hpp>
#include <darmok/stream.hpp>
#include <darmok/camera.hpp>

namespace darmok
{
	LuaEntityDefinition::LuaEntityDefinition(EntityId entity, const std::weak_ptr<Scene>& scene)
		: _entity{ entity }
		, _scene{ scene }
	{
	}

	void LuaEntityDefinition::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaEntityDefinition>("EntityDefinition", sol::no_constructor,
			"children", sol::property(&LuaEntityDefinition::getChildren),
			"components", sol::property(&LuaEntityDefinition::getComponents),
			"get_any_component", &LuaEntityDefinition::getAnyComponent,
			"for_each_child", &LuaEntityDefinition::forEachChild,
			"for_each_parent", &LuaEntityDefinition::forEachParent
		);
	}

	const EntityId& LuaEntityDefinition::getReal() const noexcept
	{
		return _entity;
	}

	std::shared_ptr<LuaEntityDefinition::Scene> LuaEntityDefinition::getScene()
	{
		if (auto scene = _scene.lock())
		{
			return scene;
		}
		throw sol::error{ "scene expired" };
	}

	bool LuaEntityDefinition::forEachChild(const sol::protected_function& callback)
	{
		auto scene = getScene();
		SceneDefinitionWrapper sceneWrapper{ *scene };
		return sceneWrapper.forEachChild(_entity, [&callback, &scene](auto entity, auto& trans) -> bool {
			auto result = callback(LuaEntityDefinition{ entity, scene }, trans);
			return LuaUtils::checkResult<bool>(result, "for each entity child");
		});
	}

	bool LuaEntityDefinition::forEachParent(const sol::protected_function& callback)
	{
		auto scene = getScene();
		SceneDefinitionWrapper sceneWrapper{ *scene };
		return sceneWrapper.forEachParent(_entity, [&callback, &scene](auto entity, auto& trans) -> bool {
			auto result = callback(LuaEntityDefinition{ entity, scene }, trans);
			return LuaUtils::checkResult<bool>(result, "for each entity parent");
		});
	}

	std::vector<LuaEntityDefinition> LuaEntityDefinition::getChildren()
	{
		auto scene = getScene();
		SceneDefinitionWrapper sceneWrapper{ *scene };
		auto result = sceneWrapper.getChildren(_entity);
		std::vector<LuaEntityDefinition> entities;
		entities.reserve(result.size());
		std::transform(result.begin(), result.end(), std::back_inserter(entities),
			[scene](const auto& entityId) { return LuaEntityDefinition{ entityId, scene }; });
		return entities;
	}

	std::vector<google::protobuf::Any*> LuaEntityDefinition::getComponents()
	{
		auto scene = getScene();
		SceneDefinitionWrapper sceneWrapper{ *scene };
		auto result = sceneWrapper.getComponents(_entity);
		std::vector<google::protobuf::Any*> comps;
		comps.reserve(result.size());
		std::transform(result.begin(), result.end(), std::back_inserter(comps),
			[](const auto& comp) { return &comp.get(); });
		return comps;
	}

	google::protobuf::Any* LuaEntityDefinition::getAnyComponent(const sol::object& type)
	{
		auto scene = getScene();
		SceneDefinitionWrapper sceneWrapper{ *scene };
		auto typeId = LuaUtils::getTypeId(type).value();
		return sceneWrapper.getComponent(_entity, typeId).ptr();
	}

	LuaSceneDefinition::LuaSceneDefinition(const std::shared_ptr<Scene>& scene)
		: _scene{ scene }
	{
	}

	const std::shared_ptr<LuaSceneDefinition::Scene>& LuaSceneDefinition::getReal() const noexcept
	{
		return _scene;
	}

	void LuaSceneDefinition::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaSceneDefinition>("SceneDefinition",
			sol::no_constructor,
			"root_entity", sol::property(&LuaSceneDefinition::getRootEntity),
			"root_entities", sol::property(&LuaSceneDefinition::getRootEntities),
			"entities", sol::property(&LuaSceneDefinition::getEntities),
			// "get_any_type_components", &LuaSceneDefinition::getAnyTypeComponents,
			"get_any_entity", &LuaSceneDefinition::getAnyEntity,
			"get_any_asset_path", &LuaSceneDefinition::getAnyAssetPath,
			"get_asset_paths", &LuaSceneDefinition::getAssetPaths,
			// "get_any_type_assets", &LuaSceneDefinition::getAnyTypeAssets,
			// "get_any_child_assets", &LuaSceneDefinition::getAnyChildAssets,
			"get_any_asset", &LuaSceneDefinition::getAnyAsset
		);
	}

	LuaEntityDefinition LuaSceneDefinition::getRootEntity()
	{
		auto entity = SceneDefinitionWrapper{ *_scene }.getRootEntity();
		return LuaEntityDefinition{ entity, _scene };
	}

	std::vector<LuaEntityDefinition> LuaSceneDefinition::getRootEntities()
	{
		auto result = SceneDefinitionWrapper{ *_scene }.getRootEntities();
		std::vector<LuaEntityDefinition> entities;
		entities.reserve(result.size());
		std::transform(result.begin(), result.end(), std::back_inserter(entities),
			[this](const auto& entityId) { return LuaEntityDefinition{ entityId, _scene }; });
		return entities;
	}

	std::vector<LuaEntityDefinition> LuaSceneDefinition::getEntities()
	{
		auto result = SceneDefinitionWrapper{ *_scene }.getEntities();
		std::vector<LuaEntityDefinition> entities;
		entities.reserve(result.size());
		std::transform(result.begin(), result.end(), std::back_inserter(entities),
			[this](const auto& entityId) { return LuaEntityDefinition{ entityId, _scene }; });
		return entities;
	}

	std::unordered_map<EntityId, google::protobuf::Any*> LuaSceneDefinition::getAnyTypeComponents(const sol::object& type)
	{
		auto typeId = LuaUtils::getTypeId(type).value();
		std::unordered_map<EntityId, google::protobuf::Any*> comps;
		if (auto result = SceneDefinitionWrapper{ *_scene }.getTypeComponents(typeId))
		{
			auto& rcomps = *result->mutable_components();
			comps.reserve(result->components_size());
			for(auto& [key, value] : rcomps)
			{
				comps.emplace(key, &value);
			}
		}
		return comps;
	}

	std::optional<LuaEntityDefinition> LuaSceneDefinition::getAnyEntity(const google::protobuf::Any& anyComp)
	{
		auto result = SceneDefinitionWrapper{ *_scene }.getEntity(anyComp);
		if(result != nullEntityId)
		{
			return LuaEntityDefinition{ result, _scene };
		}
		return std::nullopt;
	}

	std::optional<std::string> LuaSceneDefinition::getAnyAssetPath(const google::protobuf::Any& anyAsset)
	{
		auto path = SceneDefinitionWrapper{ *_scene }.getAssetPath(anyAsset);
		if (path)
		{
			return path->string();
		}
		return std::nullopt;
	}

	std::vector<std::string> LuaSceneDefinition::getAssetPaths(const sol::object& type)
	{
		auto typeId = LuaUtils::getTypeId(type).value();
		auto result = SceneDefinitionWrapper{ *_scene }.getAssetPaths(typeId);
		std::vector<std::string> paths;
		paths.reserve(result.size());
		std::transform(result.begin(), result.end(), std::back_inserter(paths),
			[](const auto& elm) { return elm.string(); });
		return paths;
	}

	LuaSceneDefinition::AssetMap LuaSceneDefinition::getAnyTypeAssets(const sol::object& type)
	{
		auto typeId = LuaUtils::getTypeId(type).value();
		auto result = SceneDefinitionWrapper{ *_scene }.getAssets(typeId);
		AssetMap assets;
		assets.reserve(result.size());
		for(auto& [key, value] : result)
		{
			assets.emplace(key.string(), value.ptr());
		}
		return assets;
	}

	LuaSceneDefinition::AssetMap LuaSceneDefinition::getAnyChildAssets(const std::filesystem::path& parentPath)
	{
		auto result = SceneDefinitionWrapper{ *_scene }.getAssets(parentPath);
		AssetMap assets;
		assets.reserve(result.size());
		for (auto& [key, value] : result)
		{
			assets.emplace(key.string(), value.ptr());
		}
		return assets;
	}

	google::protobuf::Any* LuaSceneDefinition::getAnyAsset(const std::filesystem::path& path)
	{
		return SceneDefinitionWrapper{ *_scene }.getAsset(path).ptr();
	}

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
		LuaEntityDefinition::bind(lua);
		lua.new_usertype<LuaSceneLoader>("SceneLoader", sol::constructors<
			LuaSceneLoader(), LuaSceneLoader(AssetContext&)>(),
			"parent", sol::property(&LuaSceneLoader::setParent),
			"add_component_listener", &LuaSceneLoader::addComponentListener,
			"clear_component_listeners", &LuaSceneLoader::clearComponentListeners,
			"asset_pack", sol::property(&LuaSceneLoader::getAssetPack),
			"component_load_context", sol::property(&LuaSceneLoader::getComponentLoadContext),
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

	void LuaSceneLoader::addComponentListener(const sol::object& type, const sol::function& func)
	{
		auto typeId = LuaUtils::getTypeId(type).value();

		_loader->addComponentListener([this, typeId, func](const SceneLoader::Any& compAny, Entity entity)
		{
			if (typeId != protobuf::getTypeId(compAny))
			{
				return;
			}
			auto entityDef = nullEntityId;
			if (auto sceneDef = _sceneDef.lock())
			{
				entityDef = ConstSceneDefinitionWrapper{ *sceneDef }.getEntity(compAny);
			}
			auto result = func(LuaEntityDefinition{ entityDef, _sceneDef }, LuaEntity{ entity, _scene });
			LuaUtils::checkResult(result, "scene loader component listener callback");
		});
	}

	void LuaSceneLoader::clearComponentListeners()
	{
		_loader->clearComponentListeners();
	}

	std::optional<LuaEntity> LuaSceneLoader::run(const LuaSceneDefinition& sceneDef, std::shared_ptr<Scene> scene)
	{
		_scene = scene;
		_sceneDef = sceneDef.getReal();
		auto result = (*_loader)(*sceneDef.getReal(), *scene);
		if (!result)
		{
			throw sol::error{ result.error() };
		}
		auto entity = result.value();
		if (entity == entt::null)
		{
			return std::nullopt;
		}
		return LuaEntity{ entity, scene };
	}

	IComponentLoadContext& LuaSceneLoader::getComponentLoadContext(const protobuf::Scene& sceneDef)
	{
		return _loader->getComponentLoadContext();
	}
}