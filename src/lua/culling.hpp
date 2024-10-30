#pragma once

#include "lua.hpp"
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
        static OcclusionCuller& addCameraComponent(Camera& cam) noexcept;
        static OptionalRef<OcclusionCuller>::std_t getCameraComponent(Camera& cam) noexcept;
    };

    class FrustumCuller;

    class LuaFrustumCuller final
	{
    public:
		static void bind(sol::state_view& lua) noexcept;
    private:
        static FrustumCuller& addCameraComponent(Camera& cam) noexcept;
        static OptionalRef<FrustumCuller>::std_t getCameraComponent(Camera& cam) noexcept;
    };

    class CullingDebugRenderer;

    class LuaCullingDebugRenderer final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        static CullingDebugRenderer& addCameraComponent1(Camera& cam) noexcept;
        static CullingDebugRenderer& addCameraComponent2(Camera& cam, Camera& mainCam) noexcept;
        static OptionalRef<CullingDebugRenderer>::std_t getCameraComponent(Camera& cam) noexcept;
    };
}