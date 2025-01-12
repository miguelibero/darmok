#include <darmok/camera_reflect.hpp>
#include <darmok/render_scene.hpp>

namespace darmok
{
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