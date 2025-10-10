#include "lua/protobuf.hpp"

namespace darmok
{
	namespace LuaUtils
	{
		void bindBasicProtobufField(sol::table& table, const google::protobuf::FieldDescriptor& field)
		{
			using namespace google::protobuf;
			auto fieldName = field.name();

			if (field.is_repeated())
			{

			}
			else if (field.is_map())
			{

			}
			else
			{
				switch (field.cpp_type()) {
				case FieldDescriptor::CPPTYPE_INT32:
					table[fieldName] = sol::property(
						[&field](const Message& msg) {
							return msg.GetReflection()->GetInt32(msg, &field);
						},
						[&field](Message& msg, int32_t val) {
							msg.GetReflection()->SetInt32(&msg, &field, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_INT64:
					table[fieldName] = sol::property(
						[&field](const Message& msg) {
							return msg.GetReflection()->GetInt64(msg, &field);
						},
						[&field](Message& msg, int64_t val) {
							msg.GetReflection()->SetInt64(&msg, &field, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_UINT32:
					table[fieldName] = sol::property(
						[&field](const Message& msg) {
							return msg.GetReflection()->GetUInt32(msg, &field);
						},
						[&field](Message& msg, uint32_t val) {
							msg.GetReflection()->SetUInt32(&msg, &field, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_UINT64:
					table[fieldName] = sol::property(
						[&field](const Message& msg) {
							return msg.GetReflection()->GetUInt64(msg, &field);
						},
						[&field](Message& msg, uint32_t val) {
							msg.GetReflection()->SetUInt64(&msg, &field, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_DOUBLE:
					table[fieldName] = sol::property(
						[&field](const Message& msg) {
							return msg.GetReflection()->GetDouble(msg, &field);
						},
						[&field](Message& msg, double val) {
							msg.GetReflection()->SetDouble(&msg, &field, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_FLOAT:
					table[fieldName] = sol::property(
						[&field](const Message& msg) {
							return msg.GetReflection()->GetFloat(msg, &field);
						},
						[&field](Message& msg, float val) {
							msg.GetReflection()->SetFloat(&msg, &field, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_BOOL:
					table[fieldName] = sol::property(
						[&field](const Message& msg) {
							return msg.GetReflection()->GetBool(msg, &field);
						},
						[&field](Message& msg, bool val) {
							msg.GetReflection()->SetBool(&msg, &field, val);
						}
					);
					break;
				case FieldDescriptor::CPPTYPE_ENUM:
					table[fieldName] = sol::property(
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
					table[fieldName] = sol::property(
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

		void bindBasicProtobuf(sol::table& table, const google::protobuf::Descriptor& desc)
		{
			for (int i = 0; i < desc.field_count(); ++i)
			{
				const auto* field = desc.field(i);
				if (!field)
				{
					continue;
				}
				bindBasicProtobufField(table, *field);
			}
		}
	}
}