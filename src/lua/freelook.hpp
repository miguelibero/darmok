#pragma once

#include "lua/lua.hpp"

namespace darmok
{
    class FreelookController;
    class Camera;
    class Scene;
    struct FreelookConfig;

    class LuaFreelookController final
    {
    public:
    	static void bind(sol::state_view& lua) noexcept;
    private:
        using Config = FreelookConfig;
        static std::reference_wrapper<FreelookController> addSceneComponent1(Scene& scene, Camera& cam) noexcept;
		static std::reference_wrapper<FreelookController> addSceneComponent2(Scene& scene, Camera& cam, const Config& config) noexcept;
    };
}