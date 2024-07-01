#pragma once

#include <darmok/optional_ref.hpp>
#include <sol/sol.hpp>

namespace darmok
{
    class FreelookController;
    class LuaEntity;
    class LuaTransform;
    class LuaScene;
    struct FreelookConfig;

    class LuaFreelookController final
    {
    public:
        LuaFreelookController(FreelookController& ctrl) noexcept;
        void setEnabled(bool enabled) noexcept;
        bool getEnabled() const noexcept;
    	static void bind(sol::state_view& lua) noexcept;
    private:
        OptionalRef<FreelookController> _ctrl;
        using Config = FreelookConfig;
        static LuaFreelookController addSceneComponent1(LuaScene& scene, LuaTransform& trans) noexcept;
		static LuaFreelookController addSceneComponent2(LuaScene& scene, LuaTransform& trans, const Config& config) noexcept;
    };
}