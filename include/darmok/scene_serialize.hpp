#pragma once

#include <darmok/export.h>
#include <darmok/reflect_serialize.hpp>
#include <darmok/scene_reflect.hpp>
#include <darmok/optional_ref.hpp>

#include <vector>
#include <unordered_map>

#include <cereal/types/vector.hpp>
#include <cereal/types/unordered_map.hpp>

namespace darmok
{
    struct CerealSaveEntityStorageData final
    {
        const EntitySparseSet& storage;
        OptionalRef<ISceneDelegate> delegate;

        template<typename Archive>
        void save(Archive& archive) const
        {
            auto entities = ISceneDelegate::getSerializableEntities(storage, delegate);
            archive(cereal::make_size_tag(entities.size()));
            for (auto entity : entities)
            {
                archive(CEREAL_NVP_("entity", static_cast<ENTT_ID_TYPE>(entity)));
            }
        }
    };

    struct CerealSaveEntityComponentStorageData final
    {
        entt::meta_type type;
        const EntitySparseSet& storage;
        OptionalRef<ISceneDelegate> delegate;

        template<typename Archive>
        void save(Archive& archive) const
        {
            auto entities = ISceneDelegate::getSerializableEntities(storage, delegate);
            archive(cereal::make_size_tag(entities.size()));
            for (auto entity : entities)
            {
                auto any = type.from_void(storage.value(entity));
                archive(CEREAL_NVP_("component", any));
            }
        }
    };

    struct CerealSaveComponentEntitiesData final
    {
        std::vector<CerealSaveEntityComponentStorageData>& storages;
        OptionalRef<ISceneDelegate> delegate;

        template<typename Archive>
        void save(Archive& archive) const
        {
            archive(cereal::make_size_tag(storages.size()));
            for (auto& storageData : storages)
            {
                CerealSaveEntityStorageData entitiesData{ storageData.storage, delegate };
                archive(cereal::make_map_item(storageData.type, entitiesData));
            }
        }
    };

    struct CerealSaveComponentsData final
    {
        std::vector<CerealSaveEntityComponentStorageData>& storages;

        template<typename Archive>
        void save(Archive& archive) const
        {
            archive(cereal::make_size_tag(storages.size()));
            for (auto storageData : storages)
            {
                archive(cereal::make_map_item(storageData.type, storageData));
            }
        }
    };

    class SceneComponentCerealSaveListDelegate : public ICerealReflectSaveListDelegate<ISceneComponent>
    {
    public:
        SceneComponentCerealSaveListDelegate(const Scene& scene) noexcept
            : _scene(scene)
        {
        }

        ConstSceneComponentRefs getList() const noexcept override
        {
            return _scene.getSceneComponents();
        }

        std::optional<entt::type_info> getTypeInfo(const ISceneComponent& comp) const override
        {
            return comp.getSceneComponentType();
        }

    private:
        const Scene& _scene;
    };

    template<typename Archive>
    void save(Archive& archive, const Scene& scene)
    {
        SerializeContextStack<const Scene>::push(scene);

        SceneComponentCerealSaveListDelegate sceneComps(scene);
        archive(CEREAL_NVP_("sceneComponents", sceneComps));

        auto& registry = scene.getRegistry();
        auto& entities = *registry.storage<Entity>();
        auto dlg = scene.getDelegate();
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
            storages.emplace_back(std::move(type), storage, dlg);
        }

        CerealSaveComponentEntitiesData compEntitiesData{ storages, dlg };
        archive(CEREAL_NVP_("componentEntities", compEntitiesData));
        archive(CEREAL_NVP_("entityComponents", CerealSaveComponentsData{ storages }));

        SerializeContextStack<const Scene>::pop();
    }

    struct CerealLoadEntityStorageData final
    {
        EntityStorage& storage;

        template<typename Archive>
        void load(Archive& archive)
        {
            size_t size = 0;
            archive(cereal::make_size_tag(size));
            storage.reserve(size);
            for (size_t i = 0; i < size; ++i)
            {
                darmok::Entity entity;
                archive(CEREAL_NVP_("entity", entity));
                if (entity != entt::null)
                {
                    storage.emplace(entity);
                }
            }
        }
    };

    struct CerealLoadComponentEntitiesStorageData final
    {
        EntityRegistry& registry;
        entt::meta_type type;
        std::vector<entt::meta_any> components;

        template<typename Archive>
        void load(Archive& archive)
        {
            size_t size = 0;
            archive(cereal::make_size_tag(size));
            components.reserve(size);
            for (size_t i = 0; i < size; ++i)
            {
                Entity entity;
                archive(entity);
                entt::meta_any any;
                if (entity != entt::null)
                {
                    any = SceneReflectionUtils::getEntityComponent(registry, entity, type);
                }
                components.push_back(std::move(any));
            }
        }
    };

    struct CerealLoadComponentEntitiesData final
    {
        EntityRegistry& registry;
        std::vector<CerealLoadComponentEntitiesStorageData> storages;

        template<typename Archive>
        void load(Archive& archive)
        {
            size_t size = 0;
            archive(cereal::make_size_tag(size));
            storages.reserve(size);
            for (size_t i = 0; i < size; ++i)
            {
                auto& storageData = storages.emplace_back(registry);
                archive(cereal::make_map_item(storageData.type, storageData));
            }
        }
    };

    struct CerealLoadComponentStorageData final
    {
        std::vector<entt::meta_any>& components;

        template<typename Archive>
        void load(Archive& archive)
        {
            size_t size = 0;
            archive(cereal::make_size_tag(size));
            assert(size == components.size());
            for (auto& comp : components)
            {
                archive(CEREAL_NVP_("component", comp));
            }
        }
    };

    struct CerealLoadEntityComponentsData final
    {
        std::vector<CerealLoadComponentEntitiesStorageData>& storages;

        template<typename Archive>
        void load(Archive& archive)
        {
            size_t size = 0;
            archive(cereal::make_size_tag(size));
            assert(size == storages.size());
            for (auto& storageData : storages)
            {
                CerealLoadComponentStorageData componentsData{ storageData.components };
                archive(cereal::make_map_item(storageData.type, componentsData));
            }
        }
    };

    class CerealSceneComponentLoadListDelegate : public ICerealReflectLoadListDelegate<ISceneComponent>
    {
    public:
        CerealSceneComponentLoadListDelegate(Scene& scene) noexcept
            : _scene(scene)
        {
        }

        entt::meta_any create(const entt::meta_type& type) override
        {
            return SceneReflectionUtils::getSceneComponent(_scene, type);
        }
    private:
        Scene& _scene;
    };

    template<typename Archive>
    void load(Archive& archive, Scene& scene)
    {
        SerializeContextStack<Scene>::push(scene);

        CerealSceneComponentLoadListDelegate sceneComps(scene);
        archive(CEREAL_NVP_("sceneComponents", sceneComps));

        auto& registry = scene.getRegistry();

        auto& entities = registry.storage<Entity>();
        CerealLoadEntityStorageData entitiesData{ entities };
        archive(CEREAL_NVP_("entities", entitiesData));

        size_t entityFreeList = 0;
        archive(CEREAL_NVP_("freeList", entityFreeList));
        entities.free_list(entityFreeList);

        CerealLoadComponentEntitiesData data{ registry };
        archive(CEREAL_NVP_("componentEntities", data));
        archive(CEREAL_NVP_("entityComponents", CerealLoadEntityComponentsData{ data.storages }));

        for (auto& comp : scene.getSceneComponents())
        {
            comp.get().afterLoad();
        }

        static const entt::hashed_string afterLoad{ "afterLoad" };
        for (auto& storageData : data.storages)
        {
            if (auto method = storageData.type.func(afterLoad))
            {
                for (auto& comp : storageData.components)
                {
                    method.invoke(comp);
                }
            }
        }

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