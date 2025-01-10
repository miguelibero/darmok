#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <darmok/scene.hpp>
#include <darmok/reflect.hpp>
#include <darmok/scene_reflect.hpp>
#include <darmok/serialize.hpp>
#include <entt/entt.hpp>

#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/unordered_map.hpp>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>

#include <variant>
#include <functional>

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
            bool, char,
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
                entt::meta<Archive>().template func<&doSave<Archive, T>>(key);
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
                entt::meta<Archive>().template func<&doLoad<Archive, T>>(key);
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
                archive(v.cast<const T&>());
                // save(archive, v.cast<const T&>());
                return true;
            }
            return false;
        }

        template<class Archive, typename T>
        static bool loadType(Archive& archive, entt::meta_any& v)
        {
            if (isType<T>(v))
            {
                archive(v.cast<T&>());
                // load(archive, v.cast<T&>());
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

        static bool isMinimalType(const entt::meta_any& v) noexcept
        {
            return isMinimalType(v.type());
        }

        static bool isMinimalType(const entt::meta_type& type)
        {
            if (isArithmeticType(type))
            {
                return true;
            }
            if (isType<std::string>(type))
            {
                return true;
            }
            if (isType<Entity>(type))
            {
                return true;
            }
            return false;
        }

        template<std::size_t Index = 0>
        static bool isArithmeticType(const entt::meta_type& type)
        {
            if constexpr (Index < std::variant_size_v<ArithmeticVariant>)
            {
                using T = std::variant_alternative_t<Index, ArithmeticVariant>;
                if (isType<T>(type))
                {
                    return true;
                }
                return isArithmeticType<Index + 1>(type);
            }
            return false;
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

        template<typename Archive, typename T>
        static void doSave(Archive& archive, const T& v)
        {
            // TODO: check how to save without adding a value0 tag in xml
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
        if (auto refType = darmok::SceneReflectionUtils::getEntityComponentRefType(type))
        {
            darmok::Entity entity = entt::null;
            if (auto scene = darmok::SerializeContextStack<const darmok::Scene>::tryGet())
            {
                auto ptr = darmok::ReflectionUtils::getRefPtr(v);
                if (ptr != nullptr)
                {
                    auto typeHash = refType.info().hash();
                    entity = scene->getEntity(typeHash, ptr);
                }
            }
            if (entity == entt::null)
            {
                archive(CEREAL_NVP_("entity_ref_null", true));
            }
            else
            {
                archive(
                    CEREAL_NVP_("entity_ref_null", false),
                    CEREAL_NVP_("entity_ref", entity)
                );
            }
            return;
        }

        auto typeData = type.data();
        size_t size = std::distance(typeData.begin(), typeData.end());
        archive(cereal::make_size_tag(size));
        for (auto [id, data] : typeData)
        {
            archive(cereal::make_map_item(id, v.get(id)));
        }
    }

    struct CerealLoadMetaMapItem final
    {
        entt::meta_any& any;
        entt::id_type id = 0;

        template<class Archive>
        void load(Archive& archive)
        {
            archive(any.get(id));
        }
    };

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

        if (auto refType = darmok::SceneReflectionUtils::getEntityComponentRefType(type))
        {
            bool isNull = false;
            archive(CEREAL_NVP_("entity_ref_null", isNull));
            if (isNull)
            {
                darmok::ReflectionUtils::setRef(v, nullptr);
            }
            else
            {
                darmok::Entity entity = entt::null;
                archive(CEREAL_NVP_("entity_ref", entity));
                if (entity != entt::null)
                {
                    if (auto scene = darmok::SerializeContextStack<darmok::Scene>::tryGet())
                    {
                        if (auto ptr = scene->getComponent(entity, refType.info().hash()))
                        {
                            darmok::ReflectionUtils::setRef(v, ptr);
                        }
                    }
                }
            }
            return;
        }

        size_t size;
        archive(cereal::make_size_tag(size));
        for (size_t i = 0; i < size; ++i)
        {
            CerealLoadMetaMapItem item{ v };
            archive(cereal::make_map_item(item.id, item));
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
        auto valType = v.mapped_type();
        auto keyType = v.key_type();
        auto hasStrKeys = darmok::ReflectionSerializeUtils::isType<std::string>(keyType);
        auto hasValType = (bool)valType;
        for (auto [key, val] : v)
        {
            if (!hasValType)
            {
                // set & unordered_set where the returned values are empty
                archive(key);
            }
            else if (hasStrKeys)
            {
                auto strKey = key.cast<std::string>();
                archive(cereal::make_map_item(strKey, val));
            }
            else
            {
                archive(std::make_pair(key, val));
            }
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
        auto valType = v.mapped_type();
        auto hasStrKeys = darmok::ReflectionSerializeUtils::isType<std::string>(keyType);
        auto hasValType = (bool)valType;
        for (size_t i = 0; i < size; ++i)
        {
            auto key = keyType.construct();
            auto val = valType.construct();
            if (!hasValType)
            {
                archive(key);
                val = key;
            }
            else if (hasStrKeys)
            {
                std::string strKey;
                archive(cereal::make_map_item(strKey, val));
                key = strKey;
            }
            else
            {
                archive(std::make_pair(key, val));
            }
            v.insert(key, val);
        }
    }

    inline void prologue(cereal::JSONOutputArchive& ar, const entt::meta_any& any)
    {
        if (darmok::ReflectionSerializeUtils::isMinimalType(any))
        {
            return;
        }
        ar.startNode();
    }

    inline void epilogue(cereal::JSONOutputArchive& ar, const entt::meta_any& any)
    {
        if (darmok::ReflectionSerializeUtils::isMinimalType(any))
        {
            return;
        }
        ar.finishNode();
    }

    inline void prologue(cereal::JSONInputArchive& ar, entt::meta_any& any)
    {
        if (darmok::ReflectionSerializeUtils::isMinimalType(any))
        {
            return;
        }
        ar.startNode();
    }

    inline void epilogue(cereal::JSONInputArchive& ar, entt::meta_any& any)
    {
        if (darmok::ReflectionSerializeUtils::isMinimalType(any))
        {
            return;
        }
        ar.finishNode();
    }
}

namespace darmok
{
    template<typename T>
    class DARMOK_EXPORT BX_NO_VTABLE ICerealReflectSaveListDelegate
    {
    public:
        virtual ~ICerealReflectSaveListDelegate() = default;
        virtual std::optional<entt::type_info> getTypeInfo(const T& elm) const = 0;
        virtual std::vector<std::reference_wrapper<const T>> getList() const = 0;

        template<class Archive>
        void save(Archive& archive) const
        {
            std::unordered_map<entt::id_type, entt::meta_any> elms;
            auto refList = getList();
            elms.reserve(refList.size());
            for (auto& ref : refList)
            {
                auto& elm = ref.get();
                if (auto typeInfo = getTypeInfo(elm))
                {
                    if (auto type = entt::resolve(typeInfo.value()))
                    {
                        elms.emplace(type.id(), type.from_void(&elm));
                    }
                }
            }
            cereal::save(archive, elms);
        }
    };

    class DARMOK_EXPORT BX_NO_VTABLE ICerealReflectCreateDelegate
    {
    public:
        virtual ~ICerealReflectCreateDelegate() = default;
        virtual entt::meta_any create(const entt::meta_type& type) = 0;
    };

    struct CerealReflectLoadListElementData final
    {
        ICerealReflectCreateDelegate& delegate;
        entt::meta_type type;
        entt::any obj;

        template<class Archive>
        void load(Archive& archive)
        {
            obj = delegate.create(type);
        }
    };

    template<typename T>
    class DARMOK_EXPORT BX_NO_VTABLE ICerealReflectLoadListDelegate : public ICerealReflectCreateDelegate
    {
    public:
        virtual ~ICerealReflectLoadListDelegate() = default;
        virtual void reserve(size_t size) {};
        virtual entt::meta_any create(const entt::meta_type& type) = 0;
        virtual void afterLoad() {};

        template<class Archive>
        void load(Archive& archive)
        {
            size_t size = 0;
            archive(cereal::make_size_tag(size));
            reserve(size);
            for (size_t i = 0; i < size; ++i)
            {
                CerealReflectLoadListElementData data{ *this };
                archive(cereal::make_map_item(data.type, data));
            }
            afterLoad();
        }
    };



}