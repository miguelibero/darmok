#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <darmok/scene.hpp>
#include <darmok/reflect.hpp>
#include <entt/entt.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <cereal/archives/adapters.hpp>

namespace entt
{
    struct MetaSerializeUtils final
    {
        template<class Archive>
        static bool invokeFunc(Archive& archive, const entt::meta_any& v, const char* name)
        {
            auto type = v.type();
            auto func = type.func(entt::hashed_string(name));
            if (!func)
            {
                return false;
            }
            
            if (func.is_static())
            {
                func.invoke({}, entt::forward_as_meta(archive), v);
            }
            else
            {
                func.invoke(v, entt::forward_as_meta(archive));
            }
            return true;
        }

        template<class Archive, class T, class... Types>
        static bool saveType(Archive& archive, const entt::meta_any& v)
        {
            if (doSaveType<Archive, T>(archive, v))
            {
                return true;
            }
            return saveType<Archive, Types...>(archive, v);
        }

        template<class Archive, class T, class... Types>
        static bool loadType(Archive& archive, entt::meta_any& v)
        {
            if (doLoadType<Archive, T>(archive, v))
            {
                return true;
            }
            return loadType<Archive, Types...>(archive, v);
        }

        template<class T>
        static bool isType(const entt::meta_any& v) noexcept
        {
            return isType<T>(v.type());
        }

        template<class T>
        static bool isType(const entt::meta_type& type) noexcept
        {
            return type.info().hash() == entt::type_hash<T>::value();
        }

    private:

        template<class Archive>
        static bool saveType(Archive& archive, const entt::meta_any& v)
        {
            return false;
        }

        template<class Archive>
        static bool loadType(Archive& archive, const entt::meta_any& v)
        {
            return false;
        }

        template<class Archive, class T>
        static bool doSaveType(Archive& archive, const entt::meta_any& v)
        {
            if (isType<T>(v))
            {
                archive(v.cast<const T&>());
                return true;
            }
            return false;
        }

        template<class Archive, typename T>
        static bool doLoadType(Archive& archive, entt::meta_any& v)
        {
            if (isType<T>(v))
            {
                archive(v.cast<T&>());
                return true;
            }
            return false;
        }
    };

    template<class Archive>
    void save(Archive& archive, const entt::meta_type& v)
    {
        archive(v.id());
    }

    template<class Archive>
    void load(Archive& archive, entt::meta_type& v)
    {
        entt::id_type id = 0;
        archive(id);
        v = entt::resolve(id);
    }

    template<class Archive>
    void save(Archive& archive, const entt::meta_any& v)
    {
        auto type = v.type();
        if (type.is_arithmetic())
        {
            if (MetaSerializeUtils::saveType<Archive, bool, char, char8_t>(archive, v))
            {
                return;
            }
            if (MetaSerializeUtils::saveType<Archive, char16_t, char32_t, wchar_t>(archive, v))
            {
                return;
            }
            if (MetaSerializeUtils::saveType<Archive, short, int, long, long long>(archive, v))
            {
                return;
            }
            if (MetaSerializeUtils::saveType<Archive, float, double, long double>(archive, v))
            {
                return;
            }
            throw std::invalid_argument("unknown aritmetic type");
        }
        if (MetaSerializeUtils::saveType<Archive, std::string>(archive, v))
        {
            return;
        }
        if (MetaSerializeUtils::invokeFunc(archive, v, "save"))
        {
            return;
        }
        if (MetaSerializeUtils::invokeFunc(archive, v, "serialize"))
        {
            return;
        }
        if (type.is_sequence_container())
        {
            if (auto view = v.as_sequence_container(); view)
            {
                archive(view.size());
                for (auto element : view)
                {
                    archive(element);
                }
            }
            return;
        }
        if (type.is_associative_container())
        {
            if (auto view = v.as_associative_container(); view)
            {
                archive(view.size());
                bool strKeys = MetaSerializeUtils::isType<std::string>(view.key_type());
                for (auto [key, value] : view)
                {
                    if (strKeys)
                    {
                        auto keyStr = key.cast<const std::string&>();
                        // TODO: check why this does not compile
                        // archive(cereal::make_nvp(keyStr, value));
                    }
                    else
                    {
                        archive(key, value);
                    }
                }
            }
            return;
        }

        if (auto argType = darmok::ReflectionUtils::getEntityComponentOptionalRefType(type))
        {
            darmok::Entity entity = entt::null;
            auto ptr = darmok::ReflectionUtils::getOptionalRefPtr(v);
            if (ptr != nullptr)
            {
                auto& scene = cereal::get_user_data<darmok::Scene>(archive);
                auto typeHash = argType->info().hash();
                entity = scene.getEntity(typeHash, ptr);
            }
            archive(static_cast<ENTT_ID_TYPE>(entity));
            return;
        }

        auto typeData = type.data();
        size_t size = std::distance(typeData.begin(), typeData.end());
        archive(size);
        for (auto [id, data] : typeData)
        {
            archive(id, v.get(id));
        }
    }

    template<class Archive>
    void load(Archive& archive, entt::meta_any& v)
    {
        auto type = v.type();
        if (type.is_arithmetic())
        {
            if (MetaSerializeUtils::loadType<Archive, bool, char, char8_t>(archive, v))
            {
                return;
            }
            if (MetaSerializeUtils::loadType<Archive, char16_t, char32_t, wchar_t>(archive, v))
            {
                return;
            }
            if (MetaSerializeUtils::loadType<Archive, short, int, long, long long>(archive, v))
            {
                return;
            }
            if (MetaSerializeUtils::loadType<Archive, float, double, long double>(archive, v))
            {
                return;
            }
        }
        if (MetaSerializeUtils::loadType<Archive, std::string>(archive, v))
        {
            return;
        }
        if (MetaSerializeUtils::invokeFunc(archive, v, "load"))
        {
            return;
        }
        if (MetaSerializeUtils::invokeFunc(archive, v, "serialize"))
        {
            return;
        }
        if (type.is_sequence_container())
        {
            if (auto view = v.as_sequence_container(); view)
            {
                size_t size;
                archive(size);
                for (size_t i = 0; i < size; ++i)
                {
                    auto elm = view.value_type().construct();
                    archive(elm);
                    view.insert(view.end(), elm);
                }
            }
            return;
        }
        if (type.is_associative_container())
        {
            if (auto view = v.as_associative_container(); view)
            {
                size_t size;
                archive(size);
                bool strKeys = MetaSerializeUtils::isType<std::string>(view.key_type());
                for (size_t i = 0; i < size; ++i)
                {
                    auto key = view.key_type().construct();
                    auto val = view.value_type().construct();
                    if (strKeys)
                    {
                        auto nvp = cereal::make_nvp("", val);
                        archive(nvp);
                        key.assign(nvp.name);
                        val.assign(nvp.value);
                    }
                    else
                    {
                        archive(key, val);
                    }
                    view.insert(key, val);
                }
            }
            return;
        }

        if (auto argType = darmok::ReflectionUtils::getEntityComponentOptionalRefType(type))
        {
            darmok::Entity entity;
            archive(entity);
            if (entity != entt::null)
            {
                auto& scene = cereal::get_user_data<darmok::Scene>(archive);
                if (auto ptr = scene.getComponent(entity, argType->info().hash()))
                {
                    darmok::ReflectionUtils::setOptionalRef(v, ptr);
                }
            }
            return;
        }

        size_t size;
        archive(size);
        for (size_t i = 0; i < size; ++i)
        {
            entt::id_type id;
            archive(id);
            archive(v.get(id));
        }
    }
}