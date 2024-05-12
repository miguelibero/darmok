#pragma once

#include <memory>
#include <optional>
#include <variant>
#include <unordered_map>
#include <darmok/color.hpp>
#include <darmok/scene_fwd.hpp>
#include <sol/sol.hpp>

#include "math.hpp"
#include "transform.hpp"

namespace darmok
{
    class Scene;
	class LuaScene;

	class LuaEntity final
	{
	public:
		LuaEntity(Entity entity, const std::weak_ptr<Scene>& scene) noexcept;
		std::string to_string() const noexcept;
		bool isValid() const noexcept;
		bool removeComponent(const sol::object& type);
		bool hasComponent(const sol::object& type) const;

		template<typename T, typename... Args>
		T& addComponent(Args&&... args) noexcept
		{
			return getRegistry().emplace<T>(_entity, std::forward<Args>(args)...);
		}

		template<typename T, typename L>
		std::optional<L> getComponent()
		{
			auto ptr = getRegistry().try_get<T>(_entity);
			if (ptr == nullptr)
			{
				return std::nullopt;
			}
			return L(*ptr);
		}

		LuaScene getScene() const;
		const Entity& getReal() const noexcept;

		static void configure(sol::state_view& lua) noexcept;
		[[nodiscard]] static std::optional<entt::id_type> getComponentTypeId(const sol::object& obj) noexcept;

	private:
		Entity _entity;
		std::weak_ptr<Scene> _scene;

		EntityRegistry& getRegistry();
		const EntityRegistry& getRegistry() const;
	};

	class LuaApp;

	class LuaScene final
	{
	public:
		using VarParent = std::variant<Entity, LuaTransform>;

		LuaScene(const std::shared_ptr<Scene>& scene) noexcept;
		LuaScene(LuaApp& app) noexcept;

		std::string to_string() const noexcept;
		EntityRegistry& getRegistry() noexcept;
		LuaEntity createEntity1() noexcept;
		LuaEntity createEntity2(const VarVec3& position) noexcept;
		LuaEntity createEntity3(const VarParent& parent) noexcept;
		LuaEntity createEntity4(const VarParent& parent, const VarVec3& position) noexcept;
		bool destroyEntity(const LuaEntity& entity) noexcept;

		template<typename T>
		std::optional<LuaEntity> getEntity(const T& component) noexcept
		{
			auto& storage = getRegistry().storage<T>();
			auto entity = entt::to_entity(storage, component);
			if (entity == entt::null)
			{
				return std::nullopt;
			}
			return LuaEntity(entity, _scene);
		}

		const std::shared_ptr<Scene>& getReal() const noexcept;
		std::shared_ptr<Scene>& getReal() noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<Scene> _scene;
	};
}