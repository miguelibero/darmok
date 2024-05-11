#pragma once

#include <memory>
#include <optional>
#include <variant>
#include <unordered_map>
#include <glm/glm.hpp>
#include <bx/bx.h>
#include <darmok/scene.hpp>
#include <darmok/color.hpp>
#include <darmok/optional_ref.hpp>
#include "sol.hpp"

#include "scene_fwd.hpp"
#include "transform.hpp"
#include "light.hpp"
#include "mesh.hpp"
#include "camera.hpp"
#include "math.hpp"

namespace darmok
{
	class LuaTableComponent final
	{
	public:
		void addEntry(const std::string& name, const sol::table& data);
		void setEntry(const std::string& name, const sol::table& data) noexcept;
		const sol::table& getEntry(const std::string& name) const;
		sol::table& getEntry(const std::string& name);
		bool hasEntry(const std::string& name) const noexcept;
		bool removeEntry(const std::string& name) noexcept;

	private:
		std::unordered_map<std::string, sol::table> _tables;
	};

	class LuaComponent final
	{
	public:
		LuaComponent(const std::string& name, LuaTableComponent& table) noexcept;

		const std::string& getName() const noexcept;
		sol::table& getData();
		void setData(const sol::table& data) noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		std::string _name;
		LuaTableComponent& _table;
	};

    class Scene;
	class LuaScene;

	using LuaNativeComponent = std::variant<LuaTransform, LuaCamera, LuaAmbientLight, LuaPointLight, LuaRenderable>;

	class LuaEntity final
	{
	public:
		LuaEntity(Entity entity, const std::weak_ptr<Scene>& scene) noexcept;
		std::string to_string() const noexcept;
		bool isValid() const noexcept;
		LuaComponent addLuaComponent(const std::string& name, const sol::table& data);
		LuaComponent getLuaComponent(const std::string& name);
		bool removeLuaComponent(const std::string& name);
		LuaComponent getOrAddLuaComponent(const std::string& name);
		std::optional<LuaComponent> tryGetLuaComponent(const std::string& name);
		bool hasLuaComponent(const std::string& name) const;

		template <std::size_t I = 0>
		LuaNativeComponent addNativeComponent(LuaNativeComponentType type)
		{
			if constexpr (I < std::variant_size_v<LuaNativeComponent>)
			{
				using lua_t = std::variant_alternative_t<I, LuaNativeComponent>;
				if (lua_t::native_type != type)
				{
					return addNativeComponent<I + 1>(type);
				}
				return lua_t(getRegistry().emplace<typename lua_t::native_t>(_entity));
			}
			throw std::runtime_error("missing native component");
		}

		template <std::size_t I = 0>
		LuaNativeComponent getNativeComponent(LuaNativeComponentType type)
		{
			if constexpr (I < std::variant_size_v<LuaNativeComponent>)
			{
				using lua_t = std::variant_alternative_t<I, LuaNativeComponent>;
				if (lua_t::native_type != type)
				{
					return getNativeComponent<I + 1>(type);
				}
				return lua_t(getRegistry().get<typename lua_t::native_t>(_entity));
			}
			throw std::runtime_error("missing native component");
		}

		template <std::size_t I = 0>
		LuaNativeComponent getOrAddNativeComponent(LuaNativeComponentType type)
		{
			if constexpr (I < std::variant_size_v<LuaNativeComponent>)
			{
				using lua_t = std::variant_alternative_t<I, LuaNativeComponent>;
				if (lua_t::native_type != type)
				{
					return getOrAddNativeComponent<I + 1>(type);
				}
				return lua_t(getRegistry().get_or_emplace<typename lua_t::native_t>(_entity));
			}
			throw std::runtime_error("missing native component");
		}

		template <std::size_t I = 0>
		std::optional<LuaNativeComponent> tryGetNativeComponent(LuaNativeComponentType type)
		{
			if constexpr (I < std::variant_size_v<LuaNativeComponent>)
			{
				using lua_t = std::variant_alternative_t<I, LuaNativeComponent>;
				if (lua_t::native_type != type)
				{
					return tryGetNativeComponent<I + 1>(type);
				}
				auto comp = getRegistry().try_get<typename lua_t::native_t>(_entity);
				if (comp == nullptr)
				{
					return std::nullopt;
				}
				return lua_t(*comp);

			}
			throw std::runtime_error("missing native component");
		}

		template <std::size_t I = 0>
		bool removeNativeComponent(LuaNativeComponentType type)
		{
			if constexpr (I < std::variant_size_v<LuaNativeComponent>)
			{
				using lua_t = std::variant_alternative_t<I, LuaNativeComponent>;
				if (lua_t::native_type != type)
				{
					return removeNativeComponent<I + 1>(type);
				}
				return getRegistry().remove<typename lua_t::native_t>(_entity) > 0;
			}
			return false;
		}

		template <std::size_t I = 0>
		bool hasNativeComponent(LuaNativeComponentType type) const
		{
			if constexpr (I < std::variant_size_v<LuaNativeComponent>)
			{
				using lua_t = std::variant_alternative_t<I, LuaNativeComponent>;
				if (lua_t::native_type != type)
				{
					return hasNativeComponent<I + 1>(type);
				}
				return getRegistry().try_get<typename lua_t::native_t>(_entity) != nullptr;
			}
			return false;
		}

		LuaScene getScene() const;

		const Entity& getReal() const noexcept;
		static void configure(sol::state_view& lua) noexcept;
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

		template <std::size_t I = 0>
		std::optional<LuaEntity> getEntity(const LuaNativeComponent& comp) noexcept
		{
			if constexpr (I < std::variant_size_v<LuaNativeComponent>)
			{
				using lua_t = std::variant_alternative_t<I, LuaNativeComponent>;
				auto ptr = std::get_if<lua_t>(&comp);
				if (ptr == nullptr)
				{
					return getEntity<I + 1>(comp);
				}
				using native_t = lua_t::native_t;
				auto entity = entt::to_entity(getRegistry().storage<native_t>(), ptr->getReal());
				if (entity == entt::null)
				{
					return std::nullopt;
				}
				return LuaEntity(entity, _scene);
			}
			return std::nullopt;
		}

		const std::shared_ptr<Scene>& getReal() const noexcept;
		std::shared_ptr<Scene>& getReal() noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<Scene> _scene;
	};
}