#include "lua/culling.hpp"
#include "lua/utils.hpp"
#include <darmok/culling.hpp>
#include <darmok/camera.hpp>

namespace darmok
{
    void LuaOcclusionCuller::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<OcclusionCuller>("OcclusionCuller", sol::no_constructor,
            "type_id", sol::property(&entt::type_hash<OcclusionCuller>::value),
            "add_camera_component", &LuaOcclusionCuller::addCameraComponent,
            "get_camera_component", &LuaOcclusionCuller::getCameraComponent
        );
    }

    std::reference_wrapper<OcclusionCuller> LuaOcclusionCuller::addCameraComponent(Camera& cam)
    {
        return LuaUtils::unwrapExpected(cam.addComponent<OcclusionCuller>());
    }

    OptionalRef<OcclusionCuller>::std_t LuaOcclusionCuller::getCameraComponent(Camera& cam) noexcept
    {
        return cam.getComponent<OcclusionCuller>();
    }

    void LuaFrustumCuller::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<FrustumCuller>("FrustumCuller", sol::no_constructor,
            "type_id", sol::property(&entt::type_hash<FrustumCuller>::value),
            "add_camera_component", &LuaFrustumCuller::addCameraComponent,
            "get_camera_component", &LuaFrustumCuller::getCameraComponent
        );
    }

    std::reference_wrapper<FrustumCuller> LuaFrustumCuller::addCameraComponent(Camera& cam)
    {
        return LuaUtils::unwrapExpected(cam.addComponent<FrustumCuller>());
    }

    OptionalRef<FrustumCuller>::std_t LuaFrustumCuller::getCameraComponent(Camera& cam) noexcept
    {
        return cam.getComponent<FrustumCuller>();
    }

    void LuaCullingDebugRenderer::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<CullingDebugRenderer>("CullingDebugRenderer", sol::no_constructor,
            "type_id", sol::property(&entt::type_hash<CullingDebugRenderer>::value),
            "add_camera_component", sol::overload(
                &LuaCullingDebugRenderer::addCameraComponent1,
                &LuaCullingDebugRenderer::addCameraComponent2
            ),
            "get_camera_component", &LuaCullingDebugRenderer::getCameraComponent
        );
    }

    std::reference_wrapper<CullingDebugRenderer> LuaCullingDebugRenderer::addCameraComponent1(Camera& cam)
    {
        return LuaUtils::unwrapExpected(cam.addComponent<CullingDebugRenderer>());
    }

    std::reference_wrapper<CullingDebugRenderer> LuaCullingDebugRenderer::addCameraComponent2(Camera& cam, Camera& mainCam)
    {
        return LuaUtils::unwrapExpected(cam.addComponent<CullingDebugRenderer>(mainCam));
    }

    OptionalRef<CullingDebugRenderer>::std_t LuaCullingDebugRenderer::getCameraComponent(Camera& cam) noexcept
    {
        return cam.getComponent<CullingDebugRenderer>();
    }
}