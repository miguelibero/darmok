#pragma once

#include "lua.hpp"

namespace darmok
{
    class FreelookController;
    class Camera;
    class LuaScene;
    struct FreelookConfig;

    class LuaFreelookController final
    {
    public:
    	static void bind(sol::state_view& lua) noexcept;
    private:
        using Config = FreelookConfig;
        static FreelookController& addSceneComponent1(LuaScene& scene, Camera& cam) noexcept;
		static FreelookController& addSceneComponent2(LuaScene& scene, Camera& cam, const Config& config) noexcept;
    };
}