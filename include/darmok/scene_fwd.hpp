#pragma once

#include <entt/entt.hpp>

namespace darmok
{
    /*
    using Entity = uint32_t;
    using EntityRegistry = entt::basic_registry<Entity>;
    using EntitySparseSet = entt::basic_sparse_set<Entity>;
    using EntityStorage = entt::basic_storage<Entity>;
    using SceneSnapshot = entt::basic_snapshot<EntityRegistry>;
    */
    using Entity = entt::entity;
    using EntityRegistry = entt::registry;
    using EntitySparseSet = entt::sparse_set;
    using EntityStorage = entt::storage<Entity>;
    using SceneSnapshot = entt::snapshot;
    using SceneSnapshotLoader = entt::snapshot_loader;
    using SceneContinuousLoader = entt::continuous_loader;
}