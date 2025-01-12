#pragma once

#include <entt/entt.hpp>

namespace darmok
{
    using Entity = entt::entity;
    using EntityRegistry = entt::registry;
    using EntitySparseSet = entt::sparse_set;
    using EntityStorage = entt::storage<Entity>;
    using SceneSnapshot = entt::snapshot;
    using SceneSnapshotLoader = entt::snapshot_loader;
    using SceneContinuousLoader = entt::continuous_loader;
}

namespace std
{
    template<> struct hash<entt::type_info>
    {
        std::size_t operator()(const entt::type_info& typeInfo) const noexcept
        {
            return typeInfo.hash();
        }
    };
}