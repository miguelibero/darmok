#pragma once

#include <entt/entt.hpp>

namespace darmok
{
    using Entity = uint32_t;
    using EntityRegistry = entt::basic_registry<Entity>;
    // using EntityRuntimeView = entt::basic_runtime_view<const entt::basic_sparse_set<Entity>>;
}