#pragma once

#include <sol/sol.hpp>

namespace darmok
{
    struct LuaShape final
    {
		static void bind(sol::state_view& lua) noexcept;
        static void bindRectangle(sol::state_view& lua) noexcept;
		static void bindCube(sol::state_view& lua) noexcept;
		static void bindSphere(sol::state_view& lua) noexcept;
		static void bindTriangle(sol::state_view& lua) noexcept;
		static void bindPolygon(sol::state_view& lua) noexcept;
		static void bindPlane(sol::state_view& lua) noexcept;
		static void bindCapsule(sol::state_view& lua) noexcept;
		static void bindRay(sol::state_view& lua) noexcept;
		static void bindLine(sol::state_view& lua) noexcept;
		static void bindBoundingBox(sol::state_view& lua) noexcept;
    };
}