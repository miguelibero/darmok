#pragma once
#include <darmok/export.h>
#include <type_traits>

namespace darmok
{
    struct dummy {};

    template<typename T>
    struct hasLateSave final
    {
        template<typename T, typename Archive>
        static auto test(Archive* ar) -> decltype(std::declval<const T>().lateSave(*ar), std::true_type());
        template <typename, typename>
        static std::false_type test(...);
        static const bool value = std::is_same<std::true_type, decltype(test<T, dummy>(nullptr))>::value;
    };

    template<typename T>
    struct hasLateLoad final
    {
        template<typename T, typename Archive>
        static auto test(Archive* ar) -> decltype(std::declval<T>().lateLoad(*ar), std::true_type());
        template <typename, typename>
        static std::false_type test(...);
        static const bool value = std::is_same<std::true_type, decltype(test<T, dummy>(nullptr))>::value;
    };

    template<typename T>
    struct hasLateSerialize final
    {
        template<typename T, typename Archive>
        static auto test(Archive* ar) -> decltype(std::declval<T>().lateSerialize(*ar), std::true_type());
        template <typename, typename>
        static std::false_type test(...);
        static const bool value = std::is_same<std::true_type, decltype(test<T, dummy>(nullptr))>::value;
    };

    struct DARMOK_EXPORT SerializeUtils final
    {
        template<typename T, typename Archive>
        static void lateSave(Archive& archive, const T& obj)
        {
        }

        template<typename T, typename Archive>
        static typename std::enable_if<hasLateSave<T>::value>::type lateSave(Archive& archive, const T& obj)
        {
            obj.lateSave(archive);
        }

        template<typename T, typename Archive>
        static typename std::enable_if<hasLateSerialize<T>::value>::type lateSave(Archive& archive, T& comp)
        {
            comp.lateSerialize(archive);
        }

        template<typename T, typename Archive>
        static void lateLoad(Archive& archive, const T& obj)
        {
        }

        template<typename T, typename Archive>
        static typename std::enable_if<hasLateLoad<T>::value>::type lateLoad(Archive& archive, T& comp)
        {
            comp.lateLoad(archive);
        }

        template<typename T, typename Archive>
        static typename std::enable_if<hasLateSerialize<T>::value>::type lateLoad(Archive& archive, T& comp)
        {
            comp.lateSerialize(archive);
        }
    };
}