#pragma once

#include <darmok/export.h>
#include <darmok/scene.hpp>
#include <darmok/reflect_serialize.hpp>
#include <darmok/optional_ref.hpp>
#include <cereal/archives/adapters.hpp>

namespace darmok
{
    template<typename Archive>
    void save(Archive& archive, const Scene& scene)
    {
        using StorageInfo = std::pair<entt::meta_type, std::reference_wrapper<const entt::sparse_set>>;
        std::vector<StorageInfo> storages;
        auto& registry = scene.getRegistry();

        auto& entities = *registry.storage<Entity>();
        archive(entities.size());
        archive(entities.free_list());
        for (auto itr = entities.rbegin(), last = entities.rend(); itr != last; ++itr)
        {
            archive(static_cast<ENTT_ID_TYPE>(*itr));
        }

        for (auto&& [typeHash, storage] : registry.storage())
        {
            auto type = entt::resolve(storage.type());
            if (!type)
            {
                continue;
            }
            storages.emplace_back(std::move(type), std::ref(storage));
        }
        archive(storages.size());

        for (auto& [type, storageRef] : storages)
        {
            auto& storage = storageRef.get();
            archive(type, storage.size());
            for (auto entity : storage)
            {
                archive(static_cast<ENTT_ID_TYPE>(entity));
            }
        }

        for (auto& [type, storageRef] : storages)
        {
            auto& storage = storageRef.get();
            for (auto entity : storage)
            {
                auto any = type.from_void(storage.value(entity));
                archive(any);
            }
        }
    }

    template<typename Archive>
    void load(Archive& archive, Scene& scene)
    {
        auto& registry = scene.getRegistry();

        size_t entityAmount = 0;
        archive(entityAmount);
        size_t entityFreeList = 0;
        archive(entityFreeList);

        auto& entities = registry.storage<Entity>();
        entities.reserve(entityAmount);
        for (size_t i = 0; i < entityAmount; ++i)
        {
            Entity entity;
            archive(entity);
            entities.emplace(entity);
        }
        entities.free_list(entityFreeList);

        size_t storageAmount = 0;
        archive(storageAmount);

        std::vector<entt::meta_any> components;
        for (size_t i = 0; i < storageAmount; ++i)
        {
            size_t componentAmount;
            entt::meta_type type;
            archive(type, componentAmount);
            for (size_t j = 0; j < componentAmount; ++j)
            {
                Entity entity;
                archive(entity);
                auto any = ReflectionUtils::addEntityComponent(registry, entity, type);
                components.push_back(std::move(any));
            }
        }
        for (auto& any : components)
        {
            archive(any);
        }
    }

    template<class Archive, class T>
    void save(Archive& archive, const OptionalRef<T>& v)
    {
        if (!ReflectionUtils::isEntityComponentType<T>())
        {
            return;
        }
        auto& scene = cereal::get_user_data<Scene>(archive);
        Entity entity = v ? scene.getEntity<T>(v.value()) : entt::null;
        archive(static_cast<ENTT_ID_TYPE>(entity));
    }

    template<class Archive, class T>
    void load(Archive& archive, OptionalRef<T>& v)
    {
        if (!ReflectionUtils::isEntityComponentType<T>())
        {
            return;
        }
        auto& scene = cereal::get_user_data<Scene>(archive);
        Entity entity;
        archive(entity);
        if (entity != entt::null)
        {
            v = scene.getComponent<T>(entity);
        }
    }

    template<class Archive, class T>
    void save(Archive& archive, const std::reference_wrapper<T>& v)
    {
        if (!ReflectionUtils::isEntityComponentType<T>())
        {
            return;
        }
        auto& scene = cereal::get_user_data<Scene>(archive);
        Entity entity = scene.getEntity<T>(v.get());
        archive(static_cast<ENTT_ID_TYPE>(entity));
    }

    template<class Archive, class T>
    void load(Archive& archive, std::reference_wrapper<T>& v)
    {
        if (!ReflectionUtils::isEntityComponentType<T>())
        {
            return;
        }
        auto& scene = cereal::get_user_data<Scene>(archive);
        Entity entity;
        archive(entity);
        if (entity != entt::null)
        {
            if (auto optRef = scene.getComponent<T>(entity))
            {
                v = optRef;
            }
        }
    }
}