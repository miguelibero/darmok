#pragma once

#include "lua/lua.hpp"
#include <darmok/optional_ref.hpp>

namespace darmok
{
    class Camera;
    class OcclusionCuller;

    class LuaOcclusionCuller final
	{
    public:
		static void bind(sol::state_view& lua) noexcept;
    private:
        static std::reference_wrapper<OcclusionCuller> addCameraComponent(Camera& cam);
        static OptionalRef<OcclusionCuller>::std_t getCameraComponent(Camera& cam) noexcept;
    };

    class FrustumCuller;

    class LuaFrustumCuller final
	{
    public:
		static void bind(sol::state_view& lua) noexcept;
    private:
        static std::reference_wrapper<FrustumCuller> addCameraComponent(Camera& cam);
        static OptionalRef<FrustumCuller>::std_t getCameraComponent(Camera& cam) noexcept;
    };

    class CullingDebugRenderer;

    class LuaCullingDebugRenderer final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        static std::reference_wrapper<CullingDebugRenderer> addCameraComponent1(Camera& cam);
        static std::reference_wrapper<CullingDebugRenderer> addCameraComponent2(Camera& cam, Camera& mainCam);
        static OptionalRef<CullingDebugRenderer>::std_t getCameraComponent(Camera& cam) noexcept;
    };
}