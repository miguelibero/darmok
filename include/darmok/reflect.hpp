#pragma once

#include <darmok/export.h>
#include <darmok/scene_fwd.hpp>

namespace darmok
{
	struct ReflectionUtils final
	{
		static void bind() noexcept;

		template<typename T>
		static entt::meta_factory<T> metaEntityComponent(const char* name)
		{
			return entt::meta<T>().type(entt::hashed_string{ name })
				.func<&doAddEntityComponent<T>, entt::as_ref_t>(_addEntityComponentKey);
		}

		static entt::meta_any addEntityComponent(EntityRegistry& registry, Entity entity, const entt::meta_type& type);

	private:

		static const entt::hashed_string _addEntityComponentKey;

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