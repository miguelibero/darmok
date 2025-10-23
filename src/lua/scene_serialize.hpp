#pragma once

#include "lua/lua.hpp"
#include <darmok/scene_fwd.hpp>
#include <darmok/scene_serialize.hpp>

namespace google::protobuf
{
	class Any;
};

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
		class Transform;
	};

	class LuaEntityDefinition final
	{
	public:
		using Scene = protobuf::Scene;

		LuaEntityDefinition(EntityId entity, Scene& scene);
		static void bind(sol::state_view& lua) noexcept;

		const EntityId& getReal() const noexcept;
		Scene& getScene() noexcept;

		template<typename T>
		std::shared_ptr<T> getComponent() noexcept
		{
			auto result = _scene.getComponent<T>(_entity);
			return result ? std::make_shared<T>(std::move(*result)) : nullptr;
		}

	private:
		EntityId _entity;
		SceneDefinitionWrapper _scene;
		std::vector<LuaEntityDefinition> getChildren();
		std::vector<google::protobuf::Any*> getComponents();
		google::protobuf::Any* getAnyComponent(const sol::object& type);

		bool forEachChild(const sol::protected_function& callback);
		bool forEachParent(const sol::protected_function& callback);
	};

	class LuaSceneDefinition final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:
		using AssetMap = std::unordered_map<std::string, google::protobuf::Any*>;
		static LuaEntityDefinition getRootEntity(protobuf::Scene& scene);
		static std::vector<LuaEntityDefinition> getRootEntities(protobuf::Scene& scene);
		static std::vector<LuaEntityDefinition> getEntities(protobuf::Scene& scene);
		static std::unordered_map<EntityId, google::protobuf::Any*> getAnyTypeComponents(protobuf::Scene& scene, const sol::object& type);
		static std::optional<LuaEntityDefinition> getAnyEntity(protobuf::Scene& scene, const google::protobuf::Any& anyComp);
		static std::optional<std::string> getAnyAssetPath(protobuf::Scene& scene, const google::protobuf::Any& anyAsset);
		static std::vector<std::string> getAssetPaths(protobuf::Scene& scene, const sol::object& type);
		static AssetMap getAnyTypeAssets(protobuf::Scene& scene, const sol::object& type);
		static AssetMap getAnyChildAssets(protobuf::Scene& scene, const std::filesystem::path& parentPath);
		static google::protobuf::Any* getAnyAsset(protobuf::Scene& scene, const std::filesystem::path& path);

		template<typename T>
		std::shared_ptr<T> getAsset(protobuf::Scene& scene, std::string_view path) noexcept
		{
			auto result = SceneDefinitionWrapper{ scene }.getAsset<T>(path);
			return result ? std::make_shared<T>(std::move(*result)) : nullptr;
		}
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
		IComponentLoadContext& getComponentLoadContext(const protobuf::Scene& sceneDef);
		void setParent(const LuaEntity& entity);
		void setRenderableSetup(const sol::function& func);
		AssetPack& getAssetPack();

		std::unique_ptr<SceneLoader> _loader;
		std::weak_ptr<Scene> _scene;
	};
}