#include <darmok/reflect.hpp>
#include <darmok/reflect_serialize.hpp>

namespace darmok
{
	const entt::hashed_string ReflectionUtils::_optionalRefEmptyKey = "empty";
	const entt::hashed_string ReflectionUtils::_optionalRefValueKey = "value";
	const entt::hashed_string ReflectionUtils::_optionalRefPtrKey = "ptr";
	const entt::hashed_string ReflectionUtils::_optionalRefSetKey = "set";
	const entt::hashed_string ReflectionUtils::_optionalRefResetKey = "reset";

	const entt::hashed_string ReflectionUtils::_referenceWrapperGetKey = "get";
	const entt::hashed_string ReflectionUtils::_referenceWrapperSetKey = "set";

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
}