#pragma once

#include <darmok/export.h>
#include <darmok/scene.hpp>
#include <darmok/reflect_serialize.hpp>
#include <darmok/optional_ref.hpp>
#include <cereal/types/vector.hpp>

namespace darmok
{
    struct CerealSaveEntityStorageData final
    {
        const EntitySparseSet& storage;
    };

    struct CerealSaveEntityComponentStorageData final
    {
        entt::meta_type type;
        const EntitySparseSet& storage;
    };

    struct CerealSaveComponentEntitiesData final
    {
        std::vector<CerealSaveEntityComponentStorageData>& storages;
    };

    struct CerealSaveComponentsData final
    {
        std::vector<CerealSaveEntityComponentStorageData>& storages;
    };

    template<typename Archive>
    void save(Archive& archive, const CerealSaveEntityStorageData& data)
    {
        archive(cereal::make_size_tag(data.storage.size()));
        for (auto itr = data.storage.rbegin(), last = data.storage.rend(); itr != last; ++itr)
        {
            archive(CEREAL_NVP_("entity", static_cast<ENTT_ID_TYPE>(*itr)));
        }
    }

    template<typename Archive>
    void save(Archive& archive, const CerealSaveEntityComponentStorageData& data)
    {
        archive(cereal::make_size_tag(data.storage.size()));
        for (auto itr = data.storage.rbegin(), last = data.storage.rend(); itr != last; ++itr)
        {
            Entity entity = *itr;
            auto comp = data.type.from_void(data.storage.value(entity));
            archive(CEREAL_NVP_("component", comp));
        }
    }

    template<typename Archive>
    void save(Archive& archive, const CerealSaveComponentEntitiesData& data)
    {
        archive(cereal::make_size_tag(data.storages.size()));
        for (auto& storageData : data.storages)
        {
            CerealSaveEntityStorageData entitiesData{ storageData.storage };
            archive(cereal::make_map_item(storageData.type, entitiesData));
        }
    }

    template<typename Archive>
    void save(Archive& archive, const CerealSaveComponentsData& data)
    {
        archive(cereal::make_size_tag(data.storages.size()));
        for (auto storageData : data.storages)
        {
            archive(cereal::make_map_item(storageData.type, storageData));
        }
    }

    template<typename Archive>
    void save(Archive& archive, const Scene& scene)
    {
        SerializeContextStack<const Scene>::push(scene);

        auto& registry = scene.getRegistry();
        auto& entities = *registry.storage<Entity>();
        CerealSaveEntityStorageData entitiesData{ entities };
        archive(CEREAL_NVP_("entities", entitiesData));
        archive(CEREAL_NVP_("freeList", entities.free_list()));

        std::vector<CerealSaveEntityComponentStorageData> storages;
        for (auto&& [typeHash, storage] : registry.storage())
        {
            auto type = entt::resolve(storage.type());
            if (!type)
            {
                continue;
            }
            storages.emplace_back(std::move(type), storage);
        }

        archive(CEREAL_NVP_("componentEntities", CerealSaveComponentEntitiesData{ storages }));
        archive(CEREAL_NVP_("components", CerealSaveComponentsData{ storages }));

        SerializeContextStack<const Scene>::pop();
    }

    struct CerealLoadEntityStorageData final
    {
        EntityStorage& storage;
    };

    struct CerealLoadComponentEntitiesStorageData final
    {
        EntityRegistry& registry;
        entt::meta_type type;
        std::vector<entt::meta_any> components;
    };

    struct CerealLoadComponentEntitiesData final
    {
        EntityRegistry& registry;
        std::vector<CerealLoadComponentEntitiesStorageData> storages;
    };

    struct CerealLoadComponentStorageData final
    {
        std::vector<entt::meta_any>& components;
    };

    struct CerealLoadEntityComponentsData final
    {
        std::vector<CerealLoadComponentEntitiesStorageData>& storages;
    };

    template<typename Archive>
    void load(Archive& archive, CerealLoadEntityStorageData& data)
    {
        size_t size = 0;
        archive(cereal::make_size_tag(size));
        data.storage.reserve(size);
        for (size_t i = 0; i < size; ++i)
        {
            darmok::Entity entity;
            archive(CEREAL_NVP_("entity", entity));
            data.storage.emplace(entity);
        }
    }

    template<typename Archive>
    void load(Archive& archive, CerealLoadComponentEntitiesStorageData& data)
    {
        size_t size = 0;
        archive(cereal::make_size_tag(size));
        data.components.reserve(size);
        for (size_t i = 0; i < size; ++i)
        {
            Entity entity;
            archive(entity);
            auto any = ReflectionUtils::getEntityComponent(data.registry, entity, data.type);
            data.components.push_back(std::move(any));
        }
    }

    template<typename Archive>
    void load(Archive& archive, CerealLoadComponentEntitiesData& data)
    {
        size_t size = 0;
        archive(cereal::make_size_tag(size));
        data.storages.reserve(size);
        for (size_t i = 0; i < size; ++i)
        {
            auto& storageData = data.storages.emplace_back(data.registry);
            archive(cereal::make_map_item(storageData.type, storageData));
        }
    }

    template<typename Archive>
    void load(Archive& archive, CerealLoadComponentStorageData& data)
    {
        size_t size = 0;
        archive(cereal::make_size_tag(size));
        assert(size == data.components.size());
        for (auto& comp : data.components)
        {
            archive(CEREAL_NVP_("component", comp));
        }
    }

    template<typename Archive>
    void load(Archive& archive, CerealLoadEntityComponentsData& data)
    {
        size_t size = 0;
        archive(cereal::make_size_tag(size));
        assert(size == data.storages.size());
        for (auto& storageData : data.storages)
        {
            CerealLoadComponentStorageData componentsData{ storageData.components };
            archive(cereal::make_map_item(storageData.type, componentsData));
        }
    }

    template<typename Archive>
    void load(Archive& archive, Scene& scene)
    {
        SerializeContextStack<Scene>::push(scene);

        auto& registry = scene.getRegistry();

        auto& entities = registry.storage<Entity>();
        CerealLoadEntityStorageData entitiesData{ entities };
        archive(CEREAL_NVP_("entities", entitiesData));

        size_t entityFreeList = 0;
        archive(CEREAL_NVP_("freeList", entityFreeList));
        entities.free_list(entityFreeList);

        CerealLoadComponentEntitiesData data{ registry };
        archive(CEREAL_NVP_("componentEntities", data));
        archive(CEREAL_NVP_("components", CerealLoadEntityComponentsData{ data.storages }));

        SerializeContextStack<Scene>::pop();
    }

    template<class Archive, class T>
    void save(Archive& archive, const OptionalRef<T>& v)
    {
        if (!ReflectionUtils::isEntityComponentType<T>())
        {
            return;
        }
        Entity entity = entt::null;
        if (v)
        {
            if (auto scene = SerializeContextStack<const Scene>::tryGet())
            {
                entity = scene->getEntity<T>(v.value());
            }
        }
        archive(static_cast<ENTT_ID_TYPE>(entity));
    }

    template<class Archive, class T>
    void load(Archive& archive, OptionalRef<T>& v)
    {
        if (!ReflectionUtils::isEntityComponentType<T>())
        {
            return;
        }
        Entity entity = entt::null;
        archive(entity);
        if (entity != entt::null)
        {
            if (auto scene = SerializeContextStack<Scene>::tryGet())
            {
                v = scene->getComponent<T>(entity);
            }
        }
    }
}

namespace std
{
    template<class Archive, class T>
    void save(Archive& archive, const std::reference_wrapper<T>& v)
    {
        if (!darmok::ReflectionUtils::isEntityComponentType<T>())
        {
            return;
        }
        darmok::Entity entity = entt::null;
        if (auto scene = darmok::SerializeContextStack<const darmok::Scene>::tryGet())
        {
            entity = scene->getEntity<T>(v.get());
        }
        archive(static_cast<ENTT_ID_TYPE>(entity));
    }

    template<class Archive, class T>
    void load(Archive& archive, std::reference_wrapper<T>& v)
    {
        if (!darmok::ReflectionUtils::isEntityComponentType<T>())
        {
            return;
        }
        darmok::Entity entity = entt::null;
        archive(entity);
        
        if (entity != entt::null)
        {
            if (auto scene = darmok::SerializeContextStack<darmok::Scene>::tryGet())
            {
                if (auto optRef = scene->getComponent<T>(entity))
                {
                    v = optRef;
                }
            }
        }
    }
}