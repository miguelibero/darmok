#pragma once

#include "lua/lua.hpp"
#include <darmok/protobuf.hpp>


namespace darmok
{
	namespace LuaUtils
	{
		void bindBasicProtobuf(sol::table& table, const google::protobuf::Descriptor& desc);
	}

    template<typename T>
    class LuaProtobufBinding final
    {
    private:
        sol::usertype<T> _userType;

        using Message = google::protobuf::Message;

    public:
        LuaProtobufBinding(sol::state_view& lua, std::string_view name = {}) noexcept
            : _userType{ lua.new_usertype<T>(name) }
        {
            auto desc = T::descriptor();
            if (name.empty())
            {
                name = desc->name();
            }
            auto metatable = lua.create_table_with();
            LuaUtils::bindBasicProtobuf(metatable, *desc);
            _userType[sol::metatable_key] = metatable;
        }

        template<typename P>
        LuaProtobufBinding& protobufProperty(std::string_view name) noexcept
            requires std::is_base_of_v<Message, T>
        {
            auto desc = T::descriptor();
            auto field = desc->FindFieldByName(std::string{ name });
            if (field)
            {
                return *this;
            }

            _userType[name] = sol::property(
                [field](T& msg) {
                    auto sub = msg.GetReflection()->MutableMessage(&msg, field);
                    return static_cast<P&>(*sub);
                },
                [field](T& msg, const P& val) {
                    auto sub = msg.GetReflection()->MutableMessage(&msg, field);
                    sub->CopyFrom(val);
                }
            );

            return *this;
        }
    };

    namespace LuaUtils
    {
        template<typename T>
        LuaProtobufBinding<T> newProtobuf(sol::state_view& lua, std::string_view name = {})
        {
            return { lua, name };
        }
    }
}