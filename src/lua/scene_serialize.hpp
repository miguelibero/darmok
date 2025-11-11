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
		
		LuaEntityDefinition(EntityId entity, const std::weak_ptr<Scene>& scene);
		static void bind(sol::state_view& lua) noexcept;

		const EntityId& getReal() const noexcept;
		std::shared_ptr<Scene> getScene();

		template<typename T>
		std::shared_ptr<T> getComponent() noexcept
		{
			if (auto scene = _scene.lock())
			{
				auto result = ConstSceneDefinitionWrapper{ *scene }.getComponent<T>(_entity);
				return result ? std::make_shared<T>(std::move(*result)) : nullptr;
			}
			return nullptr;
		}

	private:
		EntityId _entity;
		std::weak_ptr<Scene> _scene;
		std::vector<LuaEntityDefinition> getChildren();
		std::vector<google::protobuf::Any*> getComponents();
		google::protobuf::Any* getAnyComponent(const sol::object& type);

		bool forEachChild(const sol::protected_function& callback);
		bool forEachParent(const sol::protected_function& callback);
	};

	class LuaSceneDefinition final
	{
	public:
		using Scene = protobuf::Scene;

		LuaSceneDefinition(const std::shared_ptr<Scene>& scene);

		static void bind(sol::state_view& lua) noexcept;

		template<typename T>
		std::optional<LuaEntityDefinition> getEntity(const T& component) const noexcept
		{
			auto entityId = ConstSceneDefinitionWrapper{ *_scene }.getEntity(component);
			if (!entityId)
			{
				return std::nullopt;
			}
			return LuaEntityDefinition{ *entityId, _scene };
		}

		const std::shared_ptr<Scene>& getReal() const noexcept;

	private:
		std::shared_ptr<Scene> _scene;

		using AssetMap = std::unordered_map<std::string, google::protobuf::Any*>;
		LuaEntityDefinition getRootEntity();
		std::vector<LuaEntityDefinition> getRootEntities();
		std::vector<LuaEntityDefinition> getEntities();
		std::unordered_map<EntityId, google::protobuf::Any*> getAnyTypeComponents(const sol::object& type);
		std::optional<LuaEntityDefinition> getAnyEntity(const google::protobuf::Any& anyComp);
		std::optional<std::string> getAnyAssetPath(const google::protobuf::Any& anyAsset);
		std::vector<std::string> getAssetPaths(const sol::object& type);
		AssetMap getAnyTypeAssets(const sol::object& type);
		AssetMap getAnyChildAssets(const std::filesystem::path& parentPath);
		google::protobuf::Any* getAnyAsset(const std::filesystem::path& path);

		template<typename T>
		std::shared_ptr<T> getAsset(std::string_view path) noexcept
		{
			auto result = SceneDefinitionWrapper{ *_scene }.getAsset<T>(path);
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
		std::optional<LuaEntity> run(const LuaSceneDefinition& sceneDef, std::shared_ptr<Scene> scene);
		IComponentLoadContext& getComponentLoadContext(const protobuf::Scene& sceneDef);
		void setParent(const LuaEntity& entity);
		void addComponentListener(const sol::object& type, const sol::function& func);
		void clearComponentListeners();
		AssetPack& getAssetPack();
		LuaEntityDefinition getEntityDefinition(const LuaEntity& entity);

		std::unique_ptr<SceneLoader> _loader;
		std::weak_ptr<Scene> _scene;
	};
}