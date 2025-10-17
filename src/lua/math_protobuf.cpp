#include "lua/math.hpp"
#include "lua/protobuf.hpp"
#include <darmok/protobuf/math.pb.h>
#include <darmok/glm_serialize.hpp>

namespace darmok
{
    namespace LuaMathProtobufDetail
    {
        template<typename G, typename T>
        void bindGlm(LuaProtobufBinding<T>& binding)
        {
            binding.userType["glm"] = sol::property(
                [](const T& self) { return protobuf::convert(self); },
                [](T& self, const G& v) { self = protobuf::convert(v); }
            );
        }

	} // namespace LuaMathDetail


	void LuaMath::bindProtobuf(sol::state_view& lua) noexcept
	{
        {
            auto binding = LuaUtils::newProtobuf<protobuf::Uvec2>(lua, "Uvec2Def");
            LuaMathProtobufDetail::bindGlm<glm::uvec2>(binding);
        }
 
        {
            auto binding = LuaUtils::newProtobuf<protobuf::Vec2>(lua, "Vec2Def");
            LuaMathProtobufDetail::bindGlm<glm::vec2>(binding);
        }
        
        {
            auto binding = LuaUtils::newProtobuf<protobuf::Vec3>(lua, "Vec3Def");
            LuaMathProtobufDetail::bindGlm<glm::vec3>(binding);
        }
        {
            auto binding = LuaUtils::newProtobuf<protobuf::Vec4>(lua, "Vec4Def");
            LuaMathProtobufDetail::bindGlm<glm::vec4>(binding);
        }
        {
            auto binding = LuaUtils::newProtobuf<protobuf::Quat>(lua, "QuatDef");
            LuaMathProtobufDetail::bindGlm<glm::quat>(binding);
        }
        {
            auto binding = LuaUtils::newProtobuf<protobuf::Mat3>(lua, "Mat3Def")
                .protobufProperty<protobuf::Vec3>("col1")
                .protobufProperty<protobuf::Vec3>("col2")
                .protobufProperty<protobuf::Vec3>("col3");
            LuaMathProtobufDetail::bindGlm<glm::mat3>(binding);
        }
        {
            auto binding = LuaUtils::newProtobuf<protobuf::Mat4>(lua, "Mat4Def")
                .protobufProperty<protobuf::Vec4>("col2")
                .protobufProperty<protobuf::Vec4>("col3")
                .protobufProperty<protobuf::Vec4>("col4");
            LuaMathProtobufDetail::bindGlm<glm::mat4>(binding);
        }
        {
            auto binding = LuaUtils::newProtobuf<protobuf::Color>(lua, "ColorDef");
            LuaMathProtobufDetail::bindGlm<darmok::Color>(binding);
        }
        {
            auto binding = LuaUtils::newProtobuf<protobuf::Color3>(lua, "Color3Def");
            LuaMathProtobufDetail::bindGlm<darmok::Color3>(binding);
        }
	}
}
