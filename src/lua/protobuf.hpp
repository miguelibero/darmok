#pragma once

#include "lua/lua.hpp"
#include "lua/utils.hpp"
#include <darmok/protobuf.hpp>
#include <darmok/convert.hpp>

namespace darmok
{
    template<typename T>
	requires std::is_base_of_v<google::protobuf::Message, T>
    class LuaProtobufBinding final
    {
	private:
		sol::usertype<T> _userType;
        using Message = google::protobuf::Message;

        static std::string_view getProtobufName(std::string_view name) noexcept
        {
            if (name.empty())
            {
                name = T::descriptor()->name();
            }
            return name;
		}

		void basicProperty(const google::protobuf::FieldDescriptor& field) noexcept
		{
			using namespace google::protobuf;
			auto fieldName = field.name();

			if (field.is_repeated())
			{
				switch (field.cpp_type()) {
				case FieldDescriptor::CPPTYPE_INT32:
					_userType[fieldName] = sol::property(
						[&field](const Message& msg) {
							return protobuf::Int32FieldConverter{ field }.getVector(msg);
						},
						[&field](Message& msg, const std::vector<int32_t>& val) {
							protobuf::Int32FieldConverter{ field }.setVector(msg, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_INT64:
					_userType[fieldName] = sol::property(
						[&field](const Message& msg) {
							return protobuf::Int64FieldConverter{ field }.getVector(msg);
						},
						[&field](Message& msg, const std::vector<int64_t>& val) {
							protobuf::Int64FieldConverter{ field }.setVector(msg, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_UINT32:
					_userType[fieldName] = sol::property(
						[&field](const Message& msg) {
							return protobuf::Uint32FieldConverter{ field }.getVector(msg);
						},
						[&field](Message& msg, const std::vector<uint32_t>& val) {
							protobuf::Uint32FieldConverter{ field }.setVector(msg, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_UINT64:
					_userType[fieldName] = sol::property(
						[&field](const Message& msg) {
							return protobuf::Uint64FieldConverter{ field }.getVector(msg);
						},
						[&field](Message& msg, const std::vector<uint64_t>& val) {
							protobuf::Uint64FieldConverter{ field }.setVector(msg, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_DOUBLE:
					_userType[fieldName] = sol::property(
						[&field](const Message& msg) {
							return protobuf::DoubleFieldConverter{ field }.getVector(msg);
						},
						[&field](Message& msg, const std::vector<double>& val) {
							protobuf::DoubleFieldConverter{ field }.setVector(msg, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_FLOAT:
					_userType[fieldName] = sol::property(
						[&field](const Message& msg) {
							return protobuf::FloatFieldConverter{ field }.getVector(msg);
						},
						[&field](Message& msg, const std::vector<float> val) {
							protobuf::FloatFieldConverter{ field }.setVector(msg, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_BOOL:
					_userType[fieldName] = sol::property(
						[&field](const Message& msg) {
							return protobuf::BoolFieldConverter{ field }.getVector(msg);
						},
						[&field](Message& msg, const std::vector<bool>& val) {
							protobuf::BoolFieldConverter{ field }.setVector(msg, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_ENUM:
					_userType[fieldName] = sol::property(
						[&field](const Message& msg) {
							return protobuf::EnumFieldConverter{ field }.getVector(msg);
						},
						[&field](Message& msg, std::vector<int> val) {
							protobuf::EnumFieldConverter{ field }.setVector(msg, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_STRING:
					_userType[fieldName] = sol::property(
						[&field](const Message& msg) {
							return protobuf::StringFieldConverter{ field }.getVector(msg);
						},
						[&field](Message& msg, const std::vector<std::string>& val) {
							protobuf::StringFieldConverter{ field }.setVector(msg, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_MESSAGE:
					break;
				}
			}
			else if (field.is_map())
			{

			}
			else
			{
				switch (field.cpp_type()) {
				case FieldDescriptor::CPPTYPE_INT32:
					_userType[fieldName] = sol::property(
						[&field](const Message& msg) {
							return msg.GetReflection()->GetInt32(msg, &field);
						},
						[&field](Message& msg, int32_t val) {
							msg.GetReflection()->SetInt32(&msg, &field, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_INT64:
					_userType[fieldName] = sol::property(
						[&field](const Message& msg) {
							return msg.GetReflection()->GetInt64(msg, &field);
						},
						[&field](Message& msg, int64_t val) {
							msg.GetReflection()->SetInt64(&msg, &field, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_UINT32:
					_userType[fieldName] = sol::property(
						[&field](const Message& msg) {
							return msg.GetReflection()->GetUInt32(msg, &field);
						},
						[&field](Message& msg, uint32_t val) {
							msg.GetReflection()->SetUInt32(&msg, &field, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_UINT64:
					_userType[fieldName] = sol::property(
						[&field](const Message& msg) {
							return msg.GetReflection()->GetUInt64(msg, &field);
						},
						[&field](Message& msg, uint32_t val) {
							msg.GetReflection()->SetUInt64(&msg, &field, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_DOUBLE:
					_userType[fieldName] = sol::property(
						[&field](const Message& msg) {
							return msg.GetReflection()->GetDouble(msg, &field);
						},
						[&field](Message& msg, double val) {
							msg.GetReflection()->SetDouble(&msg, &field, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_FLOAT:
					_userType[fieldName] = sol::property(
						[&field](const Message& msg) {
							return msg.GetReflection()->GetFloat(msg, &field);
						},
						[&field](Message& msg, float val) {
							msg.GetReflection()->SetFloat(&msg, &field, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_BOOL:
					_userType[fieldName] = sol::property(
						[&field](const Message& msg) {
							return msg.GetReflection()->GetBool(msg, &field);
						},
						[&field](Message& msg, bool val) {
							msg.GetReflection()->SetBool(&msg, &field, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_ENUM:
					_userType[fieldName] = sol::property(
						[&field](const Message& msg) {
							return msg.GetReflection()->GetEnum(msg, &field);
						},
						[&field](Message& msg, int val) {
							auto enumVal = field.enum_type()->FindValueByNumber(val);
							msg.GetReflection()->SetEnum(&msg, &field, enumVal);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_STRING:
					_userType[fieldName] = sol::property(
						[&field](const Message& msg) {
							return msg.GetReflection()->GetString(msg, &field);
						},
						[&field](Message& msg, const std::string& val) {
							msg.GetReflection()->SetString(&msg, &field, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_MESSAGE:
					break;
				}
			}
		}

    public:
		LuaProtobufBinding(sol::usertype<T>&& userType, bool autoRegister = true) noexcept
			: _userType{ std::move(userType) }
		{
			if (autoRegister)
			{
				_userType["type_id"] = sol::property(&protobuf::getTypeId<T>);
				auto desc = T::descriptor();
				for (int i = 0; i < desc->field_count(); ++i)
				{
					const auto* field = desc->field(i);
					if (!field)
					{
						continue;
					}
					basicProperty(*field);
				}
			}
		}

        LuaProtobufBinding(sol::state_view& lua, std::string_view name = {}, bool autoRegister = true) noexcept
            : LuaProtobufBinding( lua.new_usertype<T>(getProtobufName(name), sol::default_constructor))
        {
        }

		LuaProtobufBinding& basicProperty(std::string_view name) noexcept
		{
			auto desc = T::descriptor();
			auto field = desc->FindFieldByName(std::string{ name });
			if (field)
			{
				basicProperty(*field);
			}
			return *this;
		}

        template<typename P>
        LuaProtobufBinding& protobufProperty(std::string_view name) noexcept
            requires std::is_base_of_v<Message, P>
        {
            auto desc = T::descriptor();
            auto field = desc->FindFieldByName(std::string{ name });
            if (!field)
            {
                return *this;
            }

			_userType[name] = sol::property(
                [field](T& msg) {
                    auto sub = msg.GetReflection()->MutableMessage(&msg, field);
                    return static_cast<P*>(sub);
                },
                [field](T& msg, const P& val) {
                    auto sub = msg.GetReflection()->MutableMessage(&msg, field);
                    sub->CopyFrom(val);
                }
            );

            return *this;
        }

		template<typename V, typename P, typename ConvertTo, typename ConvertFrom>
		LuaProtobufBinding& convertProtobufProperty(std::string_view name, ConvertTo convertTo, ConvertFrom convertFrom) noexcept
			requires std::is_base_of_v<Message, P>
				&& (std::invocable<ConvertTo, const P&> && std::same_as<std::invoke_result_t<ConvertTo, const P&>, V>)
				&& (std::invocable<ConvertFrom, const V&>&& std::same_as<std::invoke_result_t<ConvertFrom, const V&>, P>)
		{
			auto desc = T::descriptor();
			auto field = desc->FindFieldByName(std::string{ name });
			if (!field)
			{
				return *this;
			}

			_userType[name] = sol::property(
				[field, convertTo = std::move(convertTo)](T& msg) -> V {
					auto sub = msg.GetReflection()->MutableMessage(&msg, field);
					return convertTo(static_cast<P&>(*sub));
				},
				[field, convertFrom = std::move(convertFrom)](T& msg, const V& val) {
					auto sub = msg.GetReflection()->MutableMessage(&msg, field);
					sub->CopyFrom(convertFrom(val));
				}
			);

			return *this;
		}

		template<typename V, typename P = typename V::Definition>
		LuaProtobufBinding& convertProtobufProperty(std::string_view name) noexcept
			requires std::is_base_of_v<Message, P> && IsMutuallyConvertible<V, P>
		{
			return convertProtobufProperty<V, P>(name,
				convert<V, P>,
				convert<P, V>
			);
		}
    };
}