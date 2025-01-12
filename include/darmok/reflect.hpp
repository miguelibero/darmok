#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <functional>
#include <entt/entt.hpp>

namespace darmok
{
	enum class ReflectionTraits
	{
		OptionalRef,
		ReferenceWrapper,
		EntityComponent,
		SceneComponent,
		CameraComponent
	};

	struct ReflectionUtils final
	{

		template<typename T>
		static entt::meta_factory<OptionalRef<T>> metaOptionalRef(const char* name)
		{
			using R = OptionalRef<T>;
			auto refName = std::string("OptionalRef<") + name + ">";
			return entt::meta<R>().type(entt::hashed_string{ refName.c_str() })
				.traits(ReflectionTraits::OptionalRef)
				.template func<&R::ptr>(_optionalRefPtrKey)
				.template func<&setOptionalRef<T>>(_optionalRefSetKey)
				.template func<&R::value, entt::as_ref_t>(_optionalRefValueKey)
				.template func<&R::reset>(_optionalRefResetKey)
				.template func<&R::empty>(_optionalRefEmptyKey)
				;
		}

		template<typename T>
		static entt::meta_factory<std::reference_wrapper<T>> metaReferenceWrapper(const char* name)
		{
			using R = std::reference_wrapper<T>;
			auto refName = std::string("std::reference_wrapper<") + name + ">";
			return entt::meta<R>().type(entt::hashed_string{ refName.c_str() })
				.traits(ReflectionTraits::ReferenceWrapper)
				.template ctor<&createReferenceWrapper<T>>()
				.template func<&getReferenceWrapper<T>, entt::as_ref_t>(_referenceWrapperGetKey)
				.template func<&setReferenceWrapper<T>>(_referenceWrapperSetKey)
			;
		}

		static const void* getRefPtr(const entt::meta_any& any);
		static entt::meta_any getRef(const entt::meta_any& any);
		static entt::meta_type getRefType(const entt::meta_type& type);
		static void setRef(entt::meta_any& any, const void* val);

		template<typename T>
		static bool isEntityComponentType() noexcept
		{
			return entt::resolve<T>().template traits<ReflectionTraits>() == ReflectionTraits::EntityComponent;
		}

	private:

		static const entt::hashed_string _optionalRefEmptyKey;
		static const entt::hashed_string _optionalRefValueKey;
		static const entt::hashed_string _optionalRefPtrKey;
		static const entt::hashed_string _optionalRefSetKey;
		static const entt::hashed_string _optionalRefResetKey;
		static const entt::hashed_string _referenceWrapperGetKey;
		static const entt::hashed_string _referenceWrapperSetKey;

		template<typename T>
		static std::reference_wrapper<T> createReferenceWrapper() noexcept
		{
			// extremely hacky but we need a ref wrapper default constructor
			#pragma clang diagnostic push
			#pragma clang diagnostic ignored "-Wnull-dereference"
			return std::reference_wrapper<T>(*static_cast<T*>(nullptr));
			#pragma clang diagnostic pop
		}

		template<typename T>
		static T& getReferenceWrapper(std::reference_wrapper<T>& ref) noexcept
		{
			return ref.get();
		}

		template<typename T>
		static void setReferenceWrapper(std::reference_wrapper<T>& ref, const void* ptr) noexcept
		{
			if (ptr != nullptr)
			{
				ref = std::reference_wrapper<T>(*static_cast<T*>(const_cast<void*>(ptr)));
			}
		}

		template<typename T>
		static void setOptionalRef(OptionalRef<T>& ref, const void* ptr) noexcept
		{
			ref = OptionalRef<T>(static_cast<T*>(const_cast<void*>(ptr)));
		}
	};
}