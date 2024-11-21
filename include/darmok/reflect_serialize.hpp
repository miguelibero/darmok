#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <darmok/scene.hpp>
#include <darmok/reflect.hpp>
#include <entt/entt.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/utility.hpp>

#include <cereal/archives/adapters.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include <variant>

namespace darmok
{
    struct ReflectionSerializeUtils final
    {
        using OutputArchiveVariant = std::variant<
            cereal::BinaryOutputArchive,
            cereal::JSONOutputArchive,
            cereal::XMLOutputArchive,
            cereal::PortableBinaryOutputArchive
        >;

        using InputArchiveVariant = std::variant<
            cereal::BinaryInputArchive,
            cereal::JSONInputArchive,
            cereal::XMLInputArchive,
            cereal::PortableBinaryInputArchive
        >;

        using ArithmeticVariant = std::variant<
            bool, char, char8_t, char16_t, char32_t, wchar_t,
            short, int, long, long long,
            float, double, long double
        >;

        static void bind();

        template<typename T, std::size_t I = 0>
        static void metaSerialize()
        {
            metaSave<T>();
            metaLoad<T>();
        }

        template<typename T, std::size_t I = 0>
        static void metaSave()
        {
            if constexpr (I < std::variant_size_v<OutputArchiveVariant>)
            {
                using Archive = std::variant_alternative_t<I, OutputArchiveVariant>;
                auto key = _processKey.value() + entt::type_hash<T>::value();
                entt::meta<Archive>().func<&doSave<Archive, T>>(key);
                metaSave<T, I + 1>();
            }
        }

        template<typename T, std::size_t I = 0>
        static void metaLoad()
        {
            if constexpr (I < std::variant_size_v<InputArchiveVariant>)
            {
                using Archive = std::variant_alternative_t<I, InputArchiveVariant>;
                auto key = _processKey.value() + entt::type_hash<T>::value();
                entt::meta<Archive>().func<&doLoad<Archive, T>>(key);
                metaLoad<T, I + 1>();
            }
        }

        template<typename Archive, typename Any>
        static bool invokeSerialize(Archive& archive, Any& any)
        {
            auto type = any.type();
            if (!type)
            {
                return false;
            }
            auto key = _processKey.value() + type.info().hash();
            auto archiveAny = entt::forward_as_meta(archive);
            if (auto func = archiveAny.type().func(key))
            {
                func.invoke(archiveAny, any.as_ref());
                return true;
            }
            return false;
        }

        template<class Archive, class T>
        static bool saveType(Archive& archive, const entt::meta_any& v)
        {
            if (isType<T>(v))
            {
                // archive(v.cast<const T&>());
                save(archive, v.cast<const T&>());
                return true;
            }
            return false;
        }

        template<class Archive, typename T>
        static bool loadType(Archive& archive, entt::meta_any& v)
        {
            if (isType<T>(v))
            {
                // archive(v.cast<T&>());
                load(archive, v.cast<T&>());
                return true;
            }
            return false;
        }

        template<class Archive, std::size_t Index = 0>
        static bool saveArithmeticType(Archive& archive, const entt::meta_any& v)
        {
            if constexpr (Index < std::variant_size_v<ArithmeticVariant>)
            {
                using T = std::variant_alternative_t<Index, ArithmeticVariant>;
                if (saveType<Archive, T>(archive, v))
                {
                    return true;
                }
                return saveArithmeticType<Archive, Index + 1>(archive, v);
            }
            return false;
        }

        template<class Archive, std::size_t Index = 0>
        static bool loadArithmeticType(Archive& archive, entt::meta_any& v)
        {
            if constexpr (Index < std::variant_size_v<ArithmeticVariant>)
            {
                using T = std::variant_alternative_t<Index, ArithmeticVariant>;
                if (loadType<Archive, T>(archive, v))
                {
                    return true;
                }
                return loadArithmeticType<Archive, Index + 1>(archive, v);
            }
            return false;
        }

        template<class Archive, std::size_t Index = 0>
        static void saveNameValuePair(Archive& archive, const std::string& key, entt::meta_any& val)
        {
            if constexpr (Index < std::variant_size_v<ArithmeticVariant>)
            {
                using T = std::variant_alternative_t<Index, ArithmeticVariant>;
                if (doSaveNameValuePair<Archive, T>(archive, key, val))
                {
                    return;
                }
                return saveNameValuePair<Archive, Index + 1>(archive, key, val);
            }
            if (doSaveNameValuePair<Archive, std::string>(archive, key, val))
            {
                return;
            }
            save(archive, cereal::make_nvp(key, val));
        }

        template<class Archive, std::size_t KeyIndex = 0>
        static void saveMapItem(Archive& archive, entt::meta_any& key, entt::meta_any& val)
        {
            if constexpr (KeyIndex < std::variant_size_v<ArithmeticVariant>)
            {
                using Key = std::variant_alternative_t<KeyIndex, ArithmeticVariant>;
                if (isType<Key>(key))
                {
                    auto keyType = key.cast<const Key&>();
                    saveMapItem<Archive, Key>(archive, keyType, val);
                    return;
                }
                return saveMapItem<Archive, KeyIndex + 1>(archive, key, val);
            }
            if (isType<std::string>(key))
            {
                auto keyStr = key.cast<const std::string&>();
                saveMapItem<Archive, std::string>(archive, keyStr, val);
                return;
            }
            archive(cereal::make_map_item(key, val));
        }

        template<class Archive, class Key, std::size_t ValIndex = 0>
        static void saveMapItem(Archive& archive, const Key& key, const entt::meta_any& val)
        {
            if constexpr (ValIndex < std::variant_size_v<ArithmeticVariant>)
            {
                using Val = std::variant_alternative_t<ValIndex, ArithmeticVariant>;
                if (doSaveMapItem<Archive, Key, Val>(archive, key, val))
                {
                    return;
                }
                saveMapItem<Archive, Key, ValIndex + 1>(archive, key, val);
                return;
            }
            if (doSaveMapItem<Archive, Key, std::string>(archive, key, val))
            {
                return;
            }
            archive(cereal::make_map_item(key, val));
        }

        template<class T>
        static bool isType(const entt::meta_any& v) noexcept
        {
            return isType<T>(v.type());
        }

        template<class T>
        static bool isType(const entt::meta_type& type) noexcept
        {
            return type && type.info().hash() == entt::type_hash<T>::value();
        }

    private:

        static const entt::hashed_string _processKey;

        template<class Archive, class T>
        static bool doSaveNameValuePair(Archive& archive, const std::string& key, const entt::meta_any& val)
        {
            if (isType<T>(val.type()))
            {
                save(archive, cereal::make_nvp(key, val.cast<const T&>()));
                return true;
            }
            return false;
        }

        template<class Archive, class Key, class Val>
        static bool doSaveMapItem(Archive& archive, const Key& key, const entt::meta_any& val)
        {
            if (isType<Val>(val.type()))
            {
                archive(cereal::make_map_item(key, val.cast<const Val&>()));
                return true;
            }
            return false;
        }

        template<typename Archive, typename T>
        static void doSave(Archive& archive, const T& v)
        {
            archive(v);
        }

        template<typename Archive, typename T>
        static void doLoad(Archive& archive, T& v)
        {
            archive(v);
        }
    };
}

namespace entt
{
    template<class Archive>
    entt::id_type save_minimal(Archive& archive, const entt::meta_type& v)
    {
        return v.id();
    }

    template<class Archive>
    void load_minimal(Archive& archive, entt::meta_type& v, const entt::id_type& id)
    {
        v = entt::resolve(id);
    }

    // done this way to avoid duplicated error because of entt::meta_any(Type&&) constructor
    template<class Archive, class Any, std::enable_if_t<std::is_same_v<Any, entt::meta_any>, bool> = true>
    void save(Archive& archive, const Any& v)
    {
        auto type = v.type();
        if (!type)
        {
            return;
        }
        if (type.is_arithmetic())
        {
            if (darmok::ReflectionSerializeUtils::saveArithmeticType<Archive>(archive, v))
            {
                return;
            }
            throw std::invalid_argument("unknown arithmetic type");
        }
        if (darmok::ReflectionSerializeUtils::saveType<Archive, std::string>(archive, v))
        {
            return;
        }
        if (darmok::ReflectionSerializeUtils::invokeSerialize(archive, v))
        {
            return;
        }
        if (type.is_sequence_container())
        {
            if (auto view = v.as_sequence_container())
            {
                save(archive, view);
            }
            return;
        }
        if (type.is_associative_container())
        {
            if (auto view = v.as_associative_container())
            {
                save(archive, view);
            }
            return;
        }

        if (auto refType = darmok::ReflectionUtils::getEntityComponentRefType(type))
        {
            darmok::Entity entity = entt::null;
            auto ptr = darmok::ReflectionUtils::getRefPtr(v);
            if (ptr != nullptr)
            {
                auto& scene = cereal::get_user_data<darmok::Scene>(archive);
                auto typeHash = refType.info().hash();
                entity = scene.getEntity(typeHash, ptr);
            }
            archive(static_cast<ENTT_ID_TYPE>(entity));
            return;
        }

        auto typeData = type.data();
        size_t size = std::distance(typeData.begin(), typeData.end());
        archive(cereal::make_size_tag(size));
        for (auto [id, data] : typeData)
        {
            darmok::ReflectionSerializeUtils::saveMapItem(archive, id, v.get(id));
        }
    }

    template<class Archive>
    void load(Archive& archive, entt::meta_any& v)
    {
        auto type = v.type();
        if (!type)
        {
            return;
        }
        if (type.is_arithmetic())
        {
            if (darmok::ReflectionSerializeUtils::loadArithmeticType<Archive>(archive, v))
            {
                return;
            }
            throw std::invalid_argument("unknown arithmetic type");
        }
        if (darmok::ReflectionSerializeUtils::loadType<Archive, std::string>(archive, v))
        {
            return;
        }
        if (darmok::ReflectionSerializeUtils::invokeSerialize(archive, v))
        {
            return;
        }
        if (type.is_sequence_container())
        {
            if (auto view = v.as_sequence_container())
            {
                load(archive, view);
            }
            return;
        }
        if (type.is_associative_container())
        {
            if (auto view = v.as_associative_container())
            {
                load(archive, view);
            }
            return;
        }

        if (auto refType = darmok::ReflectionUtils::getEntityComponentRefType(type))
        {
            darmok::Entity entity;
            archive(entity);
            if (entity != entt::null)
            {
                auto& scene = cereal::get_user_data<darmok::Scene>(archive);
                if (auto ptr = scene.getComponent(entity, refType.info().hash()))
                {
                    darmok::ReflectionUtils::setRef(v, ptr);
                }
            }
            return;
        }

        size_t size;
        archive(cereal::make_size_tag(size));
        for (size_t i = 0; i < size; ++i)
        {
            entt::id_type id;
            archive(id);
            auto any = v.get(id);
            load(archive, any);
        }
    }

    template<class Archive>
    void save(Archive& archive, entt::meta_sequence_container v)
    {
        archive(cereal::make_size_tag(v.size()));
        for (auto element : v)
        {
            save(archive, element);
        }
    }

    template<class Archive>
    void load(Archive& archive, entt::meta_sequence_container& v)
    {
        size_t size;
        archive(cereal::make_size_tag(size));
        if (size == 0)
        {
            return;
        }
        v.reserve(size);
        auto valType = v.value_type();
        for (size_t i = 0; i < size; ++i)
        {
            auto elm = valType.construct();
            load(archive, elm);
            v.insert(v.end(), elm);
        }
    }

    template<class Archive>
    void save(Archive& archive, entt::meta_associative_container v)
    {
        archive(cereal::make_size_tag(v.size()));
        auto valType = v.value_type();
        for (auto [key, val] : v)
        {
            // hack fix for unordered_set where the value_type == key_type but the returned values are empty
            if (!val.type())
            {
                val = valType.construct();
            }
            darmok::ReflectionSerializeUtils::saveMapItem(archive, key, val);
        }
    }

    template<class Archive>
    void load(Archive& archive, entt::meta_associative_container& v)
    {
        size_t size;
        archive(cereal::make_size_tag(size));
        if (size == 0)
        {
            return;
        }
        v.reserve(size);
        auto keyType = v.key_type();
        auto valType = v.value_type();
        for (size_t i = 0; i < size; ++i)
        {
            auto key = keyType.construct();
            auto val = valType.construct();
            archive(cereal::make_map_item(key, val));
            v.insert(key, val);
        }
    }
}