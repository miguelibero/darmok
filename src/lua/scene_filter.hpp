#pragma once

#include "lua/lua.hpp"

namespace darmok
{
    struct EntityFilter;

    class LuaEntityFilter final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
        static EntityFilter create(const sol::object& value) noexcept;
    private:
        static EntityFilter operatorOr(const EntityFilter& filter, const sol::object& type) noexcept;
        static EntityFilter operatorAnd(const EntityFilter& filter, const sol::object& type) noexcept;
        static EntityFilter operatorNot(const EntityFilter& filter) noexcept;
    };
}