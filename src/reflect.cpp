#include <darmok/reflect.hpp>
#include <darmok/transform.hpp>
#include <darmok/scene.hpp>
#include <darmok/reflect_serialize.hpp>

namespace darmok
{
	const entt::hashed_string ReflectionUtils::_getEntityComponentKey = "getEntityComponent";
	const entt::hashed_string ReflectionUtils::_getEntityComponentStorageKey = "getEntityComponentStorage";

	const entt::hashed_string ReflectionUtils::_optionalRefEmptyKey = "empty";
	const entt::hashed_string ReflectionUtils::_optionalRefValueKey = "value";
	const entt::hashed_string ReflectionUtils::_optionalRefPtrKey = "ptr";
	const entt::hashed_string ReflectionUtils::_optionalRefSetKey = "set";
	const entt::hashed_string ReflectionUtils::_optionalRefResetKey = "reset";

	const entt::hashed_string ReflectionUtils::_referenceWrapperGetKey = "get";
	const entt::hashed_string ReflectionUtils::_referenceWrapperSetKey = "set";

    void ReflectionUtils::bind() noexcept
    {
		ReflectionSerializeUtils::bind();
        Transform::bindMeta();
    }

	const void* ReflectionUtils::getRefPtr(const entt::meta_any& any)
	{
		auto type = any.type();
		auto traits = type.traits<ReflectionTraits>();
		if (traits == ReflectionTraits::OptionalRef)
		{
			auto r = any.invoke(_optionalRefPtrKey);
			if (auto ptr = r.data())
			{
				return *(void**)ptr; // TODO: find less hacky way
			}
			return nullptr;
		}
		if (traits == ReflectionTraits::ReferenceWrapper)
		{
			return any.invoke(_referenceWrapperGetKey).data();
		}
		if (type.is_pointer_like())
		{
			return (*any).data();
		}
		return nullptr;
	}

	void ReflectionUtils::setRef(entt::meta_any& any, const void* val)
	{
		auto type = any.type();
		auto traits = type.traits<ReflectionTraits>();
		if (traits == ReflectionTraits::OptionalRef)
		{
			any.invoke(_optionalRefSetKey, val);
		}
		else if (traits == ReflectionTraits::ReferenceWrapper)
		{
			any.invoke(_referenceWrapperSetKey, val);
		}
		else if (type.is_pointer_like())
		{
			any.assign(type.from_void(val));
		}
	}

	entt::meta_any ReflectionUtils::getRef(const entt::meta_any& any)
	{
		auto ptr = getRefPtr(any);
		return any.type().template_arg(0u).from_void(ptr);
	}

	Entity ReflectionUtils::getEntityComponentRef(const Scene& scene, const entt::meta_any& any)
	{
		auto refType = getEntityComponentRefType(any.type());
		if (!refType)
		{
			return entt::null;
		}
		auto ptr = getRefPtr(any);
		if (ptr == nullptr)
		{
			return entt::null;
		}
		auto typeHash = refType.info().hash();
		return scene.getEntity(typeHash, ptr);
	}

	entt::meta_type ReflectionUtils::getRefType(const entt::meta_type& type)
	{
		if (type.is_pointer_like())
		{
			return type.remove_pointer();
		}
		auto traits = type.traits<ReflectionTraits>();
		if (traits == ReflectionTraits::OptionalRef || traits == ReflectionTraits::ReferenceWrapper)
		{
			return type.template_arg(0u);
		}
		return {};
	}

	entt::meta_type ReflectionUtils::getEntityComponentRefType(const entt::meta_type& type)
	{
		auto refType = getRefType(type);
		if (refType && refType.traits<ReflectionTraits>() != ReflectionTraits::EntityComponent)
		{
			return {};
		}
		return refType;
	}

	entt::meta_any ReflectionUtils::getEntityComponent(EntityRegistry& registry, Entity entity, const entt::meta_type& type)
	{
		return type.invoke(_getEntityComponentKey, {}, entt::forward_as_meta(registry), entity);
	}

	EntityRegistry::common_type& ReflectionUtils::getEntityComponentStorage(EntityRegistry& registry, const entt::meta_type& type)
	{
		return type.invoke(_getEntityComponentStorageKey, {}, entt::forward_as_meta(registry)).cast<EntityRegistry::common_type&>();
	}
}