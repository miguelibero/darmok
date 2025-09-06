#pragma once

#include "lua/lua.hpp"

#include <darmok/glm.hpp>

#include <optional>
#include <variant>

namespace darmok
{
    struct Viewport;

    using VarViewport = std::variant<Viewport, glm::uvec4, glm::uvec2, sol::table>;

    class LuaViewport final
    {
    public:
        static Viewport tableGet(const VarViewport& vp) noexcept;
        static std::optional<Viewport> tableGet(const std::optional<VarViewport>& vp) noexcept;
		static void bind(sol::state_view& lua) noexcept;
    };
}