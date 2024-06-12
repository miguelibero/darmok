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
        auto v2 = std::get_if<glm::uvec4>(&vp);
        if (v2 != nullptr)
        {
            return *v2;
        }
        auto v3 = std::get<sol::table>(vp);
        glm::uvec4 vec;
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
			"size", &Viewport::size,
			"origin", &Viewport::origin,
            "values", sol::property(&Viewport::getValues, &Viewport::setValues),
			"screen_to_viewport_point", &Viewport::viewportToScreenPoint,
			"viewport_to_screen_point", &Viewport::screenToViewportPoint,
            "project", &Viewport::project,
            "unproject", &Viewport::unproject,
            "ortho", sol::overload(
                [](const Viewport& vp, VarLuaTable<glm::vec2> center) { return Math::ortho(vp, LuaGlm::tableGet(center)); },
                [](const Viewport& vp, VarLuaTable<glm::vec2> center, float near, float far) { return Math::ortho(vp, LuaGlm::tableGet(center), near, far); }
            )
		);

    }
}