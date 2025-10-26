#include "lua/scene_serialize.hpp"
#include "lua/scene.hpp"
#include "lua/protobuf.hpp"

#include <darmok/scene_serialize.hpp>
#include <darmok/mesh.hpp>
#include <darmok/asset_pack.hpp>
#include <darmok/stream.hpp>

namespace darmok
{
	LuaEntityDefinition::LuaEntityDefinition(EntityId entity, Scene& scene)
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

	LuaEntityDefinition::Scene& LuaEntityDefinition::getScene() noexcept
	{
		return _scene.getDefinition();
	}

	bool LuaEntityDefinition::forEachChild(const sol::protected_function& callback)
	{
		auto& scene = _scene.getDefinition();
		return _scene.forEachChild(_entity, [&callback, &scene](auto entity, auto& trans) -> bool {
			auto result = callback(LuaEntityDefinition{ entity, scene }, trans);
			return LuaUtils::checkResult("for each entity child", result);
		});
	}

	bool LuaEntityDefinition::forEachParent(const sol::protected_function& callback)
	{
		auto& scene = _scene.getDefinition();
		return _scene.forEachParent(_entity, [&callback, &scene](auto entity, auto& trans) -> bool {
			auto result = callback(LuaEntityDefinition{ entity, scene }, trans);
			return LuaUtils::checkResult("for each entity parent", result);
		});
	}

	std::vector<LuaEntityDefinition> LuaEntityDefinition::getChildren()
	{
		auto result = _scene.getChildren(_entity);
		std::vector<LuaEntityDefinition> entities;
		entities.reserve(result.size());
		std::transform(result.begin(), result.end(), std::back_inserter(entities),
			[this](const auto& entityId) { return LuaEntityDefinition{ entityId, _scene.getDefinition() }; });
		return entities;
	}

	std::vector<google::protobuf::Any*> LuaEntityDefinition::getComponents()
	{
		auto result = _scene.getComponents(_entity);
		std::vector<google::protobuf::Any*> comps;
		comps.reserve(result.size());
		std::transform(result.begin(), result.end(), std::back_inserter(comps),
			[](const auto& comp) { return &comp.get(); });
		return comps;
	}

	google::protobuf::Any* LuaEntityDefinition::getAnyComponent(const sol::object& type)
	{
		auto typeId = LuaUtils::getTypeId(type).value();
		return _scene.getComponent(_entity, typeId).ptr();
	}

	void LuaSceneDefinition::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<Scene::Definition>("SceneDefinition",
			sol::constructors<Scene::Definition()>(),
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

	LuaEntityDefinition LuaSceneDefinition::getRootEntity(protobuf::Scene& scene)
	{
		auto entity = SceneDefinitionWrapper{ scene }.getRootEntity();
		return LuaEntityDefinition{ entity, scene };
	}

	std::vector<LuaEntityDefinition> LuaSceneDefinition::getRootEntities(protobuf::Scene& scene)
	{
		auto result = SceneDefinitionWrapper{ scene }.getRootEntities();
		std::vector<LuaEntityDefinition> entities;
		entities.reserve(result.size());
		std::transform(result.begin(), result.end(), std::back_inserter(entities),
			[&scene](const auto& entityId) { return LuaEntityDefinition{ entityId, scene }; });
		return entities;
	}

	std::vector<LuaEntityDefinition> LuaSceneDefinition::getEntities(protobuf::Scene& scene)
	{
		auto result = SceneDefinitionWrapper{ scene }.getEntities();
		std::vector<LuaEntityDefinition> entities;
		entities.reserve(result.size());
		std::transform(result.begin(), result.end(), std::back_inserter(entities),
			[&scene](const auto& entityId) { return LuaEntityDefinition{ entityId, scene }; });
		return entities;
	}

	std::unordered_map<EntityId, google::protobuf::Any*> LuaSceneDefinition::getAnyTypeComponents(protobuf::Scene& scene, const sol::object& type)
	{
		auto typeId = LuaUtils::getTypeId(type).value();
		std::unordered_map<EntityId, google::protobuf::Any*> comps;
		if (auto result = SceneDefinitionWrapper{ scene }.getTypeComponents(typeId))
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

	std::optional<LuaEntityDefinition> LuaSceneDefinition::getAnyEntity(protobuf::Scene& scene, const google::protobuf::Any& anyComp)
	{
		auto result = SceneDefinitionWrapper{ scene }.getEntity(anyComp);
		if(result)
		{
			return LuaEntityDefinition{ *result, scene };
		}
		return std::nullopt;
	}

	std::optional<std::string> LuaSceneDefinition::getAnyAssetPath(protobuf::Scene& scene, const google::protobuf::Any& anyAsset)
	{
		auto path = SceneDefinitionWrapper{ scene }.getAssetPath(anyAsset);
		if (path)
		{
			return path->string();
		}
		return std::nullopt;
	}

	std::vector<std::string> LuaSceneDefinition::getAssetPaths(protobuf::Scene& scene, const sol::object& type)
	{
		auto typeId = LuaUtils::getTypeId(type).value();
		auto result = SceneDefinitionWrapper{ scene }.getAssetPaths(typeId);
		std::vector<std::string> paths;
		paths.reserve(result.size());
		std::transform(result.begin(), result.end(), std::back_inserter(paths),
			[&scene](const auto& elm) { return elm.string(); });
		return paths;
	}

	LuaSceneDefinition::AssetMap LuaSceneDefinition::getAnyTypeAssets(protobuf::Scene& scene, const sol::object& type)
	{
		auto typeId = LuaUtils::getTypeId(type).value();
		auto result = SceneDefinitionWrapper{ scene }.getAssets(typeId);
		AssetMap assets;
		assets.reserve(result.size());
		for(auto& [key, value] : result)
		{
			assets.emplace(key.string(), value.ptr());
		}
		return assets;
	}

	LuaSceneDefinition::AssetMap LuaSceneDefinition::getAnyChildAssets(protobuf::Scene& scene, const std::filesystem::path& parentPath)
	{
		auto result = SceneDefinitionWrapper{ scene }.getAssets(parentPath);
		AssetMap assets;
		assets.reserve(result.size());
		for (auto& [key, value] : result)
		{
			assets.emplace(key.string(), value.ptr());
		}
		return assets;
	}

	google::protobuf::Any* LuaSceneDefinition::getAnyAsset(protobuf::Scene& scene, const std::filesystem::path& path)
	{
		return SceneDefinitionWrapper{ scene }.getAsset(path).ptr();
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
			"renderable_setup", sol::property(&LuaSceneLoader::setRenderableSetup),
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

	void LuaSceneLoader::setRenderableSetup(const sol::function& func)
	{
		_loader->addComponentListener<Renderable>([this, func](const Renderable::Definition& def, Entity entity)
		{
			auto result = func(def, LuaEntity{ entity, _scene });
			LuaUtils::throwResult(result, "renderable setup callback");
		});
	}

	std::optional<LuaEntity> LuaSceneLoader::run(const protobuf::Scene& sceneDef, std::shared_ptr<Scene> scene)
	{
		_scene = scene;
		auto result = (*_loader)(sceneDef, *scene);
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