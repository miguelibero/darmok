#pragma once

#include <darmok/export.h>
#include <darmok/camera.hpp>
#include <darmok/reflect.hpp>
#include <entt/entt.hpp>

namespace darmok
{
	struct CameraReflectionUtils final
	{
		template<typename T>
		static entt::meta_factory<T> metaCameraComponent(const char* name)
		{
			return entt::meta<T>().type(entt::hashed_string{ name })
				.traits(ReflectionTraits::CameraComponent)
				.template func<&doGetCameraComponent<T>, entt::as_ref_t>(_getCameraComponentKey);
		}

		static entt::meta_any getCameraComponent(Camera& cam, const entt::meta_type& type);
		static std::vector<entt::meta_any> getCameraComponents(Camera& cam);

	private:

		static const entt::hashed_string _getCameraComponentKey;

		template<typename T>
		static T& doGetCameraComponent(Camera& cam)
		{
			return cam.getOrAddComponent<T>();
		}
	};
}