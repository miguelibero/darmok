#include <darmok/reflect.hpp>
#include <darmok/reflect_serialize.hpp>
#include <darmok/scene_reflect.hpp>
#include <darmok/camera_reflect.hpp>

#include <darmok/transform.hpp>
#include <darmok/shadow.hpp>
#include <darmok/light.hpp>
#include <darmok/culling.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/render_shape.hpp>

namespace darmok
{
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
		Camera::bindMeta();
		PointLight::bindMeta();
		DirectionalLight::bindMeta();
		SpotLight::bindMeta();
		AmbientLight::bindMeta();
		CubeRenderable::bindMeta();
		SphereRenderable::bindMeta();
		CapsuleRenderable::bindMeta();
		PlaneRenderable::bindMeta();

		ShadowRenderer::bindMeta();
		LightingRenderComponent::bindMeta();
		ForwardRenderer::bindMeta();
		FrustumCuller::bindMeta();
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

	const entt::hashed_string SceneReflectionUtils::_getEntityComponentKey = "getEntityComponent";
	const entt::hashed_string SceneReflectionUtils::_getEntityComponentStorageKey = "getEntityComponentStorage";
	const entt::hashed_string SceneReflectionUtils::_getSceneComponentKey = "getSceneComponent";

	Entity SceneReflectionUtils::getEntityComponentRef(const Scene& scene, const entt::meta_any& any)
	{
		auto refType = getEntityComponentRefType(any.type());
		if (!refType)
		{
			return entt::null;
		}
		auto ptr = ReflectionUtils::getRefPtr(any);
		if (ptr == nullptr)
		{
			return entt::null;
		}
		auto typeHash = refType.info().hash();
		return scene.getEntity(typeHash, ptr);
	}

	entt::meta_type SceneReflectionUtils::getEntityComponentRefType(const entt::meta_type& type)
	{
		auto refType = ReflectionUtils::getRefType(type);
		if (refType && refType.traits<ReflectionTraits>() != ReflectionTraits::EntityComponent)
		{
			return {};
		}
		return refType;
	}

	entt::meta_any SceneReflectionUtils::getEntityComponent(EntityRegistry& registry, Entity entity, const entt::meta_type& type)
	{
		return type.invoke(_getEntityComponentKey, {}, entt::forward_as_meta(registry), entity);
	}

	EntityRegistry::common_type& SceneReflectionUtils::getEntityComponentStorage(EntityRegistry& registry, const entt::meta_type& type)
	{
		return type.invoke(_getEntityComponentStorageKey, {}, entt::forward_as_meta(registry)).cast<EntityRegistry::common_type&>();
	}

	entt::meta_any SceneReflectionUtils::getSceneComponent(Scene& scene, const entt::meta_type& type)
	{
		return type.invoke(_getSceneComponentKey, {}, entt::forward_as_meta(scene));
	}

	std::vector<entt::meta_any> SceneReflectionUtils::getSceneComponents(Scene& scene)
	{
		std::vector<entt::meta_any> comps;
		for (auto& compRef : scene.getSceneComponents())
		{
			auto& comp = compRef.get();
			if (auto typeInfo = comp.getSceneComponentType())
			{
				if (auto type = entt::resolve(typeInfo.value()))
				{
					comps.push_back(type.from_void(&comp));
				}
			}
		}
		return comps;
	}

	std::vector<entt::meta_any> SceneReflectionUtils::getEntityComponents(Scene& scene, Entity entity)
	{
		std::vector<entt::meta_any> comps;
		for (auto [typeInfo, ptr] : scene.getComponents(entity))
		{
			if (auto type = entt::resolve(typeInfo))
			{
				comps.push_back(type.from_void(ptr));
			}
		}
		return comps;
	}

	const entt::hashed_string CameraReflectionUtils::_getCameraComponentKey = "getCameraComponent";

	entt::meta_any CameraReflectionUtils::getCameraComponent(Camera& cam, const entt::meta_type& type)
	{
		return type.invoke(_getCameraComponentKey, {}, entt::forward_as_meta(cam));
	}

	std::vector<entt::meta_any> CameraReflectionUtils::getCameraComponents(Camera& cam)
	{
		std::vector<entt::meta_any> comps;
		for (auto& compRef : cam.getComponents())
		{
			auto& comp = compRef.get();
			if (auto typeInfo = comp.getCameraComponentType())
			{
				if (auto type = entt::resolve(typeInfo.value()))
				{
					comps.push_back(type.from_void(&comp));
				}
			}
		}
		return comps;
	}
}