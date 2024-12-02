#pragma once

#include <darmok/export.h>
#include <darmok/scene.hpp>
#include <darmok/reflect.hpp>
#include <entt/entt.hpp>

namespace darmok
{
	struct SceneReflectionUtils final
	{
		template<typename T>
		static entt::meta_factory<T> metaEntityComponent(const char* name)
		{
			ReflectionUtils::metaOptionalRef<T>(name);
			ReflectionUtils::metaReferenceWrapper<T>(name);

			return entt::meta<T>().type(entt::hashed_string{ name })
				.traits(ReflectionTraits::EntityComponent)
				.func<&doGetEntityComponent<T>, entt::as_ref_t>(_getEntityComponentKey)
				.func<&doGetEntityComponentStorage<T>, entt::as_ref_t>(_getEntityComponentStorageKey);
		}

		template<typename T>
		static entt::meta_factory<T> metaSceneComponent(const char* name)
		{
			return entt::meta<T>().type(entt::hashed_string{ name })
				.traits(ReflectionTraits::SceneComponent)
				.func<&doGetSceneComponent<T>, entt::as_ref_t>(_getSceneComponentKey);
		}

		static entt::meta_any getEntityComponent(EntityRegistry& registry, Entity entity, const entt::meta_type& type);
		static EntityRegistry::common_type& getEntityComponentStorage(EntityRegistry& registry, const entt::meta_type& type);
		static entt::meta_type getEntityComponentRefType(const entt::meta_type& type);
		static Entity getEntityComponentRef(const Scene& scene, const entt::meta_any& any);

		static entt::meta_any getSceneComponent(Scene& scene, const entt::meta_type& type);

	private:

		static const entt::hashed_string _getEntityComponentKey;
		static const entt::hashed_string _getEntityComponentStorageKey;
		static const entt::hashed_string _getSceneComponentKey;

		template<typename T>
		static EntityRegistry::common_type& doGetEntityComponentStorage(EntityRegistry& registry)
		{
			return registry.storage<T>();
		}

		template<typename T>
		static T& doGetEntityComponent(EntityRegistry& registry, Entity entity)
		{
			return registry.get_or_emplace<T>(entity);
		}

		template<typename T>
		static T& doGetSceneComponent(Scene& scene)
		{
			return scene.getOrAddSceneComponent<T>();
		}
	};
}