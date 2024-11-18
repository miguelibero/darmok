#include <darmok/reflect.hpp>
#include <darmok/transform.hpp>

namespace darmok
{
	const entt::hashed_string ReflectionUtils::_addEntityComponentKey = "addEntityComponent";

    void ReflectionUtils::bind() noexcept
    {
        Transform::bindMeta();
    }

	entt::meta_any ReflectionUtils::addEntityComponent(EntityRegistry& registry, Entity entity, const entt::meta_type& type)
	{
		return type.invoke(_addEntityComponentKey, {}, entt::forward_as_meta(registry), entity).as_ref();
	}
}