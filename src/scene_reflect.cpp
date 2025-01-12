#include <darmok/scene_reflect.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/shadow.hpp>
#include <darmok/light.hpp>
#include <darmok/culling.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/render_scene.hpp>

namespace darmok
{
    void SceneReflectionUtils::bind() noexcept
    {
		ReflectionSerializeUtils::bind();
        
		Transform::bindMeta();
		Camera::bindMeta();
		PointLight::bindMeta();
		DirectionalLight::bindMeta();
		SpotLight::bindMeta();
		AmbientLight::bindMeta();
		Renderable::bindMeta();

		ShadowRenderer::bindMeta();
		LightingRenderComponent::bindMeta();
		ForwardRenderer::bindMeta();
		FrustumCuller::bindMeta();
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
}