#pragma once

#include <darmok/export.h>
#include <darmok/scene_fwd.hpp>
#include <darmok/optional_ref.hpp>
#include <functional>

namespace darmok
{
	enum class ReflectionTraits
	{
		OptionalRef,
		ReferenceWrapper,
		EntityComponent
	};

	class Scene;

	struct ReflectionUtils final
	{
		static void bind() noexcept;

		template<typename T>
		static entt::meta_factory<OptionalRef<T>> metaOptionalRef(const char* name)
		{
			using R = OptionalRef<T>;
			auto refName = std::string("OptionalRef<") + name + ">";
			return entt::meta<R>().type(entt::hashed_string{ refName.c_str() })
				.traits(ReflectionTraits::OptionalRef)
				.func<&R::ptr>(_optionalRefPtrKey)
				.func<&setOptionalRef<T>>(_optionalRefSetKey)
				.func<&R::value, entt::as_ref_t>(_optionalRefValueKey)
				.func<&R::reset>(_optionalRefResetKey)
				.func<&R::empty>(_optionalRefEmptyKey)
				;
		}

		template<typename T>
		static entt::meta_factory<std::reference_wrapper<T>> metaReferenceWrapper(const char* name)
		{
			using R = std::reference_wrapper<T>;
			auto refName = std::string("std::reference_wrapper<") + name + ">";
			return entt::meta<R>().type(entt::hashed_string{ refName.c_str() })
				.ctor<&createReferenceWrapper<T>>()
				.traits(ReflectionTraits::ReferenceWrapper)
				.func<&R::get, entt::as_ref_t>(_referenceWrapperGetKey)
				.func<&setReferenceWrapper<T>>(_referenceWrapperSetKey)
			;
		}

		template<typename T>
		static entt::meta_factory<T> metaEntityComponent(const char* name)
		{
			metaOptionalRef<T>(name);
			metaReferenceWrapper<T>(name);

			return entt::meta<T>().type(entt::hashed_string{ name })
				.traits(ReflectionTraits::EntityComponent)
				.func<&doAddEntityComponent<T>, entt::as_ref_t>(_addEntityComponentKey)
				.func<&doGetEntityComponentStorage<T>, entt::as_ref_t>(_getEntityComponentStorageKey);
		}

		static entt::meta_any addEntityComponent(EntityRegistry& registry, Entity entity, const entt::meta_type& type);
		static EntityRegistry::common_type& getEntityComponentStorage(EntityRegistry& registry, const entt::meta_type& type);
		
		static const void* getRefPtr(const entt::meta_any& any);
		static entt::meta_any getRef(const entt::meta_any& any);
		static Entity getEntityComponentRef(const Scene& scene, const entt::meta_any& any);
		static entt::meta_type getRefType(const entt::meta_type& type);
		static entt::meta_type getEntityComponentRefType(const entt::meta_type& type);
		static void setRef(entt::meta_any& any, const void* val);
	private:

		static const entt::hashed_string _optionalRefEmptyKey;
		static const entt::hashed_string _optionalRefValueKey;
		static const entt::hashed_string _optionalRefPtrKey;
		static const entt::hashed_string _optionalRefSetKey;
		static const entt::hashed_string _optionalRefResetKey;
		static const entt::hashed_string _referenceWrapperGetKey;
		static const entt::hashed_string _referenceWrapperSetKey;

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

		template<typename T>
		static std::reference_wrapper<T> createReferenceWrapper() noexcept
		{
			// hacky but we need a ref wrapper default constructor
			return std::reference_wrapper<T>(*static_cast<T*>(nullptr));
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