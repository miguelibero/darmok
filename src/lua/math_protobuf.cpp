#include "lua/math.hpp"
#include "lua/protobuf.hpp"
#include <darmok/protobuf/math.pb.h>
#include <darmok/glm_serialize.hpp>

namespace darmok
{
    namespace LuaMathProtobufDetail
    {
        template<typename G, typename T>
        void bindGlm(sol::usertype<T>& userType)
        {
            userType["glm"] = sol::property(
                [](const T& self) { return convert<G>(self); },
                [](T& self, const G& v) { self = convert<T>(v); }
            );
        }

	} // namespace LuaMathDetail


	void LuaMath::bindProtobuf(sol::state_view& lua) noexcept
	{
        {
            auto def = lua.new_usertype<protobuf::Uvec2>("Uvec2Def", sol::default_constructor);
            LuaMathProtobufDetail::bindGlm<glm::uvec2>(def);
            LuaProtobufBinding{ std::move(def) };
        }
        {
            auto def = lua.new_usertype<protobuf::Vec2>("Vec2Def", sol::default_constructor);
            LuaMathProtobufDetail::bindGlm<glm::vec2>(def);
            LuaProtobufBinding{ std::move(def) };
        }
        {
            auto def = lua.new_usertype<protobuf::Vec3>("Vec3Def", sol::default_constructor);
            LuaMathProtobufDetail::bindGlm<glm::vec3>(def);
            LuaProtobufBinding{ std::move(def) };
        }
        {
            auto def = lua.new_usertype<protobuf::Vec4>("Vec4Def", sol::default_constructor);
            LuaMathProtobufDetail::bindGlm<glm::vec4>(def);
            LuaProtobufBinding{ std::move(def) };
        }
        {
            auto def = lua.new_usertype<protobuf::Quat>("QuatDef", sol::default_constructor);
            LuaMathProtobufDetail::bindGlm<glm::quat>(def);
            LuaProtobufBinding{ std::move(def) };
        }
        {
            auto def = lua.new_usertype<protobuf::Mat3>("Mat3Def", sol::default_constructor);
            LuaMathProtobufDetail::bindGlm<glm::mat3>(def);
            LuaProtobufBinding{ std::move(def) }
                .convertProtobufProperty<glm::vec3, protobuf::Vec3>("col1")
                .convertProtobufProperty<glm::vec3, protobuf::Vec3>("col2")
                .convertProtobufProperty<glm::vec3, protobuf::Vec3>("col3")
                ;
        }
        {
            auto def = lua.new_usertype<protobuf::Mat4>("Mat4Def", sol::default_constructor);
            LuaMathProtobufDetail::bindGlm<glm::mat4>(def);
            LuaProtobufBinding{ std::move(def) }
                .convertProtobufProperty<glm::vec4, protobuf::Vec4>("col1")
                .convertProtobufProperty<glm::vec4, protobuf::Vec4>("col2")
                .convertProtobufProperty<glm::vec4, protobuf::Vec4>("col3")
                .convertProtobufProperty<glm::vec4, protobuf::Vec4>("col4")
                ;
        }
        {
            auto def = lua.new_usertype<protobuf::Color>("ColorDef", sol::default_constructor);
            LuaMathProtobufDetail::bindGlm<darmok::Color>(def);
            LuaProtobufBinding{ std::move(def) };
        }
        {
            auto def = lua.new_usertype<protobuf::Color3>("Color3Def", sol::default_constructor);
            LuaMathProtobufDetail::bindGlm<darmok::Color3>(def);
            LuaProtobufBinding{ std::move(def) };
        }
	}
}
