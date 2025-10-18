#include "lua/scene_serialize.hpp"
#include "lua/scene.hpp"
#include "lua/protobuf.hpp"

#include <darmok/scene_serialize.hpp>
#include <darmok/mesh.hpp>
#include <darmok/asset_pack.hpp>
#include <darmok/stream.hpp>

namespace darmok
{
	void LuaSceneDefinition::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<Scene::Definition>("SceneDefinition",
			sol::constructors<Scene::Definition()>(),
			"root_entity", sol::property(
				[](Scene::Definition& def)
				{
					auto entities = SceneDefinitionWrapper{ def }.getRootEntities();
					if (entities.empty())
					{
						return nullEntityId;
					}
					return entities.back();
				}),
			"root_entities", sol::property(
				[](Scene::Definition& def)
				{
					return SceneDefinitionWrapper{ def }.getRootEntities();
				}),
			"entities", sol::property(
				[](Scene::Definition& def)
				{
					return SceneDefinitionWrapper{ def }.getEntities();
				}),
			"get_children", [](Scene::Definition& def, EntityId entity)
				{
					return SceneDefinitionWrapper{ def }.getChildren(entity);
				},
			"get_type_components", [](Scene::Definition& def, const sol::object& type)
				{
					auto typeId = LuaUtils::getTypeId(type).value();
					std::vector<const google::protobuf::Any*> comps;
					if (auto result = SceneDefinitionWrapper{ def }.getTypeComponents(typeId))
					{
						for (auto& comp : result->components())
						{
							comps.push_back(&comp.second);
						}
					}
					return comps;
				},
			"get_components", [](Scene::Definition& def, EntityId entity)
				{
					std::vector<google::protobuf::Any*> comps;
					auto result = SceneDefinitionWrapper{ def }.getComponents(entity);
					for (auto& comp : result)
					{
						comps.push_back(&comp.get());
					}
					return comps;
				},
			"get_component", [](Scene::Definition& def, EntityId entity, const sol::object& type)
				{
					auto typeId = LuaUtils::getTypeId(type).value();
					return SceneDefinitionWrapper{ def }.getComponent(entity, typeId).ptr();
				},
			"get_entity", [](Scene::Definition& def, const google::protobuf::Any& anyComp)
				{
					return SceneDefinitionWrapper{ def }.getEntity(anyComp);
				},
			"get_asset_path", [](Scene::Definition& def, const google::protobuf::Any& anyAsset) -> std::string
				{
					auto path = SceneDefinitionWrapper{ def }.getAssetPath(anyAsset);
					if(path)
					{
						return path->string();
					}
					return {};
				},
			"get_asset_paths", [](Scene::Definition& def, const sol::object& type)
				{
					auto typeId = LuaUtils::getTypeId(type).value();
					std::vector<std::string> paths;
					auto result = SceneDefinitionWrapper{ def }.getAssetPaths(typeId);
					for(auto& path : result)
					{
						paths.push_back(path.string());
					}
					return paths;
				},
			/*
			"get_type_assets", [](Scene::Definition& def, const sol::object& type)
				{
					auto typeId = LuaUtils::getTypeId(type).value();
					std::unordered_map<std::string, SceneDefinitionWrapper::Any*> assets;
					auto result = SceneDefinitionWrapper{ def }.getAssets(typeId);
					for(auto& [path, any] : result)
					{
						assets.emplace(path.string(), any.ptr());
					}
					return assets;
				},
			"get_child_assets", [](Scene::Definition& def, const std::filesystem::path& parentPath)
				{
					std::unordered_map<std::string, SceneDefinitionWrapper::Any*> assets;
					auto result = SceneDefinitionWrapper{ def }.getAssets(parentPath);
					for (auto& [path, any] : result)
					{
						assets.emplace(path.string(), any.ptr());
					}
					return assets;
				},
				*/
			"get_asset", [](Scene::Definition& def, const std::filesystem::path& path)
				{
					return SceneDefinitionWrapper{ def }.getAsset(path).ptr();
				}
		);
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