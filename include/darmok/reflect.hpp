#pragma once

#include <darmok/export.h>
#include <darmok/scene_fwd.hpp>
#include <darmok/optional_ref.hpp>
#include <optional>

namespace darmok
{
	enum class ReflectionTraits
	{
		OptionalRef,
		EntityComponent
	};

	class Scene;

	struct ReflectionUtils final
	{
		static void bind() noexcept;

		template<typename T>
		static entt::meta_factory<OptionalRef<T>> metaOptionalRef(const char* name)
		{
			auto optRefName = std::string("OptionalRef<") + name + ">";
			return entt::meta<OptionalRef<T>>().type(entt::hashed_string{ optRefName.c_str() })
				.traits(ReflectionTraits::OptionalRef)
				.func<&OptionalRef<T>::ptr>(entt::hashed_string{ _optionalRefPtrKey })
				.func<&OptionalRef<T>::set, entt::as_void_t>(entt::hashed_string{ _optionalRefSetKey })
				.func<&OptionalRef<T>::value, entt::as_ref_t>(entt::hashed_string{ _optionalRefValueKey })
				.func<&OptionalRef<T>::reset>(entt::hashed_string{ _optionalRefResetKey })
				.func<&OptionalRef<T>::empty>(entt::hashed_string{ _optionalRefEmptyKey });
		}

		template<typename T>
		static entt::meta_factory<T> metaEntityComponent(const char* name)
		{
			metaOptionalRef<T>(name);

			return entt::meta<T>().type(entt::hashed_string{ name })
				.traits(ReflectionTraits::EntityComponent)
				.func<&doAddEntityComponent<T>, entt::as_ref_t>(_addEntityComponentKey)
				.func<&doGetEntityComponentStorage<T>, entt::as_ref_t>(_getEntityComponentStorageKey);
		}

		static entt::meta_any addEntityComponent(EntityRegistry& registry, Entity entity, const entt::meta_type& type);
		static EntityRegistry::common_type& getEntityComponentStorage(EntityRegistry& registry, const entt::meta_type& type);
		
		static const void* getOptionalRefPtr(const entt::meta_any& any);
		static entt::meta_any getOptionalRef(const entt::meta_any& any);
		static Entity getEntityComponentOptionalRef(const Scene& scene, const entt::meta_any& any);
		static std::optional<entt::meta_type> getEntityComponentOptionalRefType(const entt::meta_type& type);
		static void setOptionalRef(entt::meta_any& any, const void* val);
	private:

		static const entt::hashed_string _optionalRefEmptyKey;
		static const entt::hashed_string _optionalRefValueKey;
		static const entt::hashed_string _optionalRefPtrKey;
		static const entt::hashed_string _optionalRefSetKey;
		static const entt::hashed_string _optionalRefResetKey;

		static const entt::hashed_string _addEntityComponentKey;
		static const entt::hashed_string _getEntityComponentStorageKey;

		template<typename T>
		static EntityRegistry::common_type& doGetEntityComponentStorage(EntityRegistry& registry)
		{
			return registry.storage<T>();
		}

		template<typename T>
		static T& doAddEntityComponent(EntityRegistry& registry, Entity entity)
		{
			auto& entities = registry.storage<Entity>();
			if (!entities.contains(entity))
			{
				entities.emplace(entity);
			}
			return registry.storage<T>().emplace(entity);
		}
	};
}