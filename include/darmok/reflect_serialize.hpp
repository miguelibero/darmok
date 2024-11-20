#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <darmok/scene.hpp>
#include <darmok/reflect.hpp>
#include <entt/entt.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>

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
            return type && type.info().hash() == entt::type_hash<T>::value();
        }

    private:

        static const entt::hashed_string _processKey;

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
        if (!type)
        {
            return;
        }
        if (type.is_arithmetic())
        {
            if (darmok::ReflectionSerializeUtils::saveType<Archive, bool, char, char8_t>(archive, v))
            {
                return;
            }
            if (darmok::ReflectionSerializeUtils::saveType<Archive, char16_t, char32_t, wchar_t>(archive, v))
            {
                return;
            }
            if (darmok::ReflectionSerializeUtils::saveType<Archive, short, int, long, long long>(archive, v))
            {
                return;
            }
            if (darmok::ReflectionSerializeUtils::saveType<Archive, float, double, long double>(archive, v))
            {
                return;
            }
            throw std::invalid_argument("unknown aritmetic type");
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
            if (auto view = v.as_associative_container())
            {
                archive(view.size());
                bool strKeys = darmok::ReflectionSerializeUtils::isType<std::string>(view.key_type());
                auto valType = view.value_type();
                for (auto [key, val] : view)
                {
                    // hack fix for unordered_set where the value_type == key_type but the returned values are empty
                    if (val.type() != valType)
                    {
                        val = valType.construct();
                    }
                    bool saved = false;
                    if constexpr (!std::is_same<cereal::BinaryOutputArchive, Archive>::value)
                    {
                        if (strKeys)
                        {
                            auto& keyStr = key.cast<const std::string&>();
                            archive(cereal::make_nvp(keyStr.c_str(), val.as_ref()));
                            saved = true;
                        }
                    }
                    if(!saved)
                    {
                        archive(key, val);
                    }
                }
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
        if (!type)
        {
            return;
        }
        if (type.is_arithmetic())
        {
            if (darmok::ReflectionSerializeUtils::loadType<Archive, bool, char, char8_t>(archive, v))
            {
                return;
            }
            if (darmok::ReflectionSerializeUtils::loadType<Archive, char16_t, char32_t, wchar_t>(archive, v))
            {
                return;
            }
            if (darmok::ReflectionSerializeUtils::loadType<Archive, short, int, long, long long>(archive, v))
            {
                return;
            }
            if (darmok::ReflectionSerializeUtils::loadType<Archive, float, double, long double>(archive, v))
            {
                return;
            }
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
                size_t size;
                archive(size);
                if (size == 0)
                {
                    return;
                }
                view.reserve(size);
                auto valType = view.value_type();
                for (size_t i = 0; i < size; ++i)
                {
                    auto elm = valType.construct();
                    archive(elm);
                    view.insert(view.end(), elm);
                }
            }
            return;
        }
        if (type.is_associative_container())
        {
            if (auto view = v.as_associative_container())
            {
                size_t size;
                archive(size);
                if (size == 0)
                {
                    return;
                }
                view.reserve(size);
                auto keyType = view.key_type();
                auto valType = view.value_type();
                bool strKeys = darmok::ReflectionSerializeUtils::isType<std::string>(view.key_type());
                for (size_t i = 0; i < size; ++i)
                {
                    auto key = keyType.construct();
                    auto val = valType.construct();
                    bool loaded = false;
                    if constexpr (!std::is_same<cereal::BinaryInputArchive, Archive>::value)
                    {
                        if (strKeys)
                        {
                            auto nvp = cereal::make_nvp("", val.as_ref());
                            archive(nvp);
                            loaded = true;
                            key.assign(nvp.name);
                        }
                    }
                    if(!loaded)
                    {
                        archive(key, val);
                    }
                    view.insert(key, val);
                }
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
        archive(size);
        for (size_t i = 0; i < size; ++i)
        {
            entt::id_type id;
            archive(id);
            archive(v.get(id));
        }
    }
}