#include <darmok/reflect_serialize.hpp>
#include <darmok/glm_serialize.hpp>

using namespace entt::literals;

namespace darmok
{
    const entt::hashed_string ReflectionSerializeUtils::_processKey = "process";

    void ReflectionSerializeUtils::bind()
    {
        entt::meta<glm::vec2>().type("glm::vec2"_hs);
        entt::meta<glm::vec3>().type("glm::vec3"_hs);
        entt::meta<glm::vec4>().type("glm::vec4"_hs);
        entt::meta<glm::ivec2>().type("glm::ivec2"_hs);
        entt::meta<glm::ivec3>().type("glm::ivec3"_hs);
        entt::meta<glm::ivec4>().type("glm::ivec4"_hs);
        entt::meta<glm::uvec2>().type("glm::uvec2"_hs);
        entt::meta<glm::uvec3>().type("glm::uvec3"_hs);
        entt::meta<glm::uvec4>().type("glm::uvec4"_hs);
        entt::meta<glm::quat>().type("glm::quat"_hs);
        entt::meta<glm::mat3>().type("glm::mat3"_hs);
        entt::meta<glm::mat4>().type("glm::mat4"_hs);

        metaSerialize<glm::vec2>();
        metaSerialize<glm::vec3>();
        metaSerialize<glm::vec4>();
        metaSerialize<glm::ivec2>();
        metaSerialize<glm::ivec3>();
        metaSerialize<glm::ivec4>();
        metaSerialize<glm::uvec2>();
        metaSerialize<glm::uvec3>();
        metaSerialize<glm::uvec4>();
        metaSerialize<glm::quat>();
        metaSerialize<glm::mat3>();
        metaSerialize<glm::mat4>();
    }
}