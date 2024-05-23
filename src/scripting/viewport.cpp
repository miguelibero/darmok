#include "viewport.hpp"
#include "glm.hpp"
#include <darmok/viewport.hpp>

namespace darmok
{
    Viewport LuaViewport::tableGet(const VarViewport& vp) noexcept
    {
        auto v1 = std::get_if<Viewport>(&vp);
        if (v1 != nullptr)
        {
            return *v1;
        }
        auto v2 = std::get_if<glm::ivec4>(&vp);
        if (v2 != nullptr)
        {
            return *v2;
        }
        auto v3 = std::get<sol::table>(vp);
        glm::ivec4 vec;
        LuaGlm::tableInit(vec, v3);
        return vec;
    }

    std::optional<Viewport> LuaViewport::tableGet(const std::optional<VarViewport>& vp) noexcept
    {
        if (vp)
        {
            return tableGet(vp.value());
        }
        return std::nullopt;
    }

    void LuaViewport::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<Viewport>("Viewport", sol::no_constructor,
			"size", sol::property(&Viewport::getSize, &Viewport::setSize),
			"origin", sol::property(&Viewport::getOrigin, &Viewport::setOrigin),
			"screen_to_viewport_point", &Viewport::viewportToScreenPoint,
			"viewport_to_screen_point", &Viewport::screenToViewportPoint
		);

    }
}