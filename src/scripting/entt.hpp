#pragma once

#include <entt/entt.hpp>
#include <entt/meta/factory.hpp>
#include <entt/meta/resolve.hpp>
#include "sol.hpp"
#include <optional>

namespace darmok
{
    struct LuaEntt final
    {
        template <typename... Args>
        inline auto invokeMetaFunction(entt::meta_type metaType,
                                    entt::id_type functionId, Args&&... args) {
            if (!metaType)
            {
                // TODO: Warning message
            }
            else if (auto&& metaFunction = metaType.func(functionId); metaFunction)
            {
                return metaFunction.invoke({}, std::forward<Args>(args)...);
            }
            return entt::meta_any{};
        }

        template <typename... Args>
        inline auto invokeMetaFunction(entt::id_type typeId, entt::id_type functionId,
                                    Args&&...args)
        {
            return invokeMetaFunction(entt::resolve(typeId), functionId, std::forward<Args>(args)...);
        }

        [[nodiscard]] static std::optional<entt::id_type> getTypeId(const sol::object& obj) noexcept;
    };
}