#include <darmok/reflect.hpp>
#include <darmok/transform.hpp>
#include <darmok/scene.hpp>
#include <darmok/reflect_serialize.hpp>

namespace darmok
{
	const entt::hashed_string ReflectionUtils::_addEntityComponentKey = "addEntityComponent";
	const entt::hashed_string ReflectionUtils::_getEntityComponentStorageKey = "getEntityComponentStorage";

	const entt::hashed_string ReflectionUtils::_optionalRefEmptyKey = "empty";
	const entt::hashed_string ReflectionUtils::_optionalRefValueKey = "value";
	const entt::hashed_string ReflectionUtils::_optionalRefPtrKey = "ptr";
	const entt::hashed_string ReflectionUtils::_optionalRefSetKey = "set";
	const entt::hashed_string ReflectionUtils::_optionalRefResetKey = "reset";

    void ReflectionUtils::bind() noexcept
    {
		ReflectionSerializeUtils::bind();
        Transform::bindMeta();
    }

	const void* ReflectionUtils::getOptionalRefPtr(const entt::meta_any& any)
	{
		auto r = any.invoke(_optionalRefPtrKey);
		return *(void**)r.data(); // TODO: find less hacky way
	}

	void ReflectionUtils::setOptionalRef(entt::meta_any& any, const void* val)
	{
		if (!val)
		{
			any.invoke(_optionalRefResetKey);
			return;
		}
		auto valAny = any.type().template_arg(0u).from_void(val);
		any.invoke(_optionalRefSetKey, valAny);
	}

	entt::meta_any ReflectionUtils::getOptionalRef(const entt::meta_any& any)
	{
		auto type = any.type();
		if (!type.is_template_specialization() || type.traits<ReflectionTraits>() != ReflectionTraits::OptionalRef)
		{
			return entt::meta_any{ std::in_place_type<void> };
		}
		auto ptr = getOptionalRefPtr(any);
		return type.template_arg(0u).from_void(ptr);
	}

	Entity ReflectionUtils::getEntityComponentOptionalRef(const Scene& scene, const entt::meta_any& any)
	{
		auto argType = getEntityComponentOptionalRefType(any.type());
		if (!argType)
		{
			return entt::null;
		}
		auto ptr = getOptionalRefPtr(any);
		if (ptr == nullptr)
		{
			return entt::null;
		}
		auto typeHash = argType->info().hash();
		return scene.getEntity(typeHash, ptr);
	}

	std::optional<entt::meta_type> ReflectionUtils::getEntityComponentOptionalRefType(const entt::meta_type& type)
	{
		if (type.traits<ReflectionTraits>() != ReflectionTraits::OptionalRef)
		{
			return std::nullopt;
		}
		if (!type.is_template_specialization())
		{
			return std::nullopt;
		}
		auto argType = type.template_arg(0u);
		if (argType.traits<ReflectionTraits>() != ReflectionTraits::EntityComponent)
		{
			return std::nullopt;
		}
		return argType;
	}

	entt::meta_any ReflectionUtils::addEntityComponent(EntityRegistry& registry, Entity entity, const entt::meta_type& type)
	{
		return type.invoke(_addEntityComponentKey, {}, entt::forward_as_meta(registry), entity);
	}

	EntityRegistry::common_type& ReflectionUtils::getEntityComponentStorage(EntityRegistry& registry, const entt::meta_type& type)
	{
		return type.invoke(_getEntityComponentStorageKey, {}, entt::forward_as_meta(registry)).cast<EntityRegistry::common_type&>();
	}
}