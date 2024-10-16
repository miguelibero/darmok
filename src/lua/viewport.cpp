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
        auto v3 = std::get_if<glm::uvec2>(&vp);
        if (v3 != nullptr)
        {
            return *v3;
        }
        auto v4 = std::get<sol::table>(vp);
        if (v4.size() == 4)
        {
            glm::uvec4 vec;
            LuaGlm::tableInit(vec, v4);
            return vec;
        }
        if (v4.size() == 2)
        {
            glm::uvec2 vec;
            LuaGlm::tableInit(vec, v4);
            return vec;
        }
        return Viewport();
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
        lua.new_usertype<Viewport>("Viewport",
            sol::factories(
                []()
                {
                    return Viewport();
                },
                [](const VarLuaTable<glm::uvec2>& size, const VarLuaTable<glm::uvec2>& origin)
                {
                    return Viewport(LuaGlm::tableGet(size), LuaGlm::tableGet(origin));
                },
                [](const VarLuaTable<glm::uvec2>& size)
                {
                    return Viewport(LuaGlm::tableGet(size));
                },
                [](const VarLuaTable<glm::uvec4>& values)
                {
                    return Viewport(LuaGlm::tableGet(values));
                },
                [](glm::uint x, glm::uint y, glm::uint w, glm::uint h)
                {
                    return Viewport(x, y, w, h);
                }
            ),
			"size", &Viewport::size,
			"origin", &Viewport::origin,
            "values", sol::property(&Viewport::getValues, &Viewport::setValues),
			"viewport_to_screen_point", &Viewport::viewportToScreenPoint,
			"screen_to_viewport_point", &Viewport::screenToViewportPoint,
            "project", &Viewport::project,
            "unproject", &Viewport::unproject,
            "ortho", sol::overload(
                [](const Viewport& vp, VarLuaTable<glm::vec2> center) { return vp.ortho(LuaGlm::tableGet(center)); },
                [](const Viewport& vp, VarLuaTable<glm::vec2> center, float near, float far) { return vp.ortho(LuaGlm::tableGet(center), near, far); }
            )
		);

    }
}