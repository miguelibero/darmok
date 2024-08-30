#pragma once

#ifdef _DEBUG

#include <sol/sol.hpp>

namespace darmok
{
    class LuaScene;
    class RmluiDebuggerComponent;
    struct RmluiDebuggerComponentConfig;

    class LuaRmluiDebuggerComponent final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        using Config = RmluiDebuggerComponentConfig;

        static RmluiDebuggerComponent& addSceneComponent1(LuaScene& scene) noexcept;
        static RmluiDebuggerComponent& addSceneComponent2(LuaScene& scene, const Config& config) noexcept;
    };
}

#endif