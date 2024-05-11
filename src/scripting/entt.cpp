#include "entt.hpp"

namespace darmok
{
    std::optional<entt::id_type> LuaEntt::getTypeId(const sol::object& obj) noexcept
    {
        switch (obj.get_type())
        {
        case sol::type::number:
            return obj.template as<entt::id_type>();
        case sol::type::table:
            sol::table tab = obj;
            auto f = tab["type_id"].get<sol::function>();
            if (f.valid())
            {
                return f().get<entt::id_type>();
            }
            break;
        }
        return std::nullopt;
    }
}