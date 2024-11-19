#include <darmok/reflect_serialize.hpp>

namespace darmok
{
    const entt::hashed_string ReflectionSerializeUtils::_serializeKey = "serialize";
    const entt::hashed_string ReflectionSerializeUtils::_saveKey = "save";
    const entt::hashed_string ReflectionSerializeUtils::_loadKey = "load";

    void ReflectionSerializeUtils::bind()
    {
    }
}