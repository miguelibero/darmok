#pragma once

#include <entt/entt.hpp>

namespace darmok
{
    using Entity = uint32_t;
    using EntityRegistry = entt::basic_registry<Entity>;
    using EntitySparseSet = entt::basic_sparse_set<Entity>;
    using EntityStorage = entt::basic_storage<Entity>;
}