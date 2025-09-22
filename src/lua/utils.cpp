#include "lua/utils.hpp"
#include <sstream>
#include <iostream>
#include <functional>
#include <bx/debug.h>
#include <bx/string.h>
#include <darmok/stream.hpp>
#include <darmok/utils.hpp>

namespace darmok
{
	namespace LuaUtils
	{
		bool isArray(const sol::table& table)  noexcept
		{
			if (table.empty())
			{
				return true;
			}
			auto key = (*table.cbegin()).first;
			if (key.get_type() != sol::type::number)
			{
				return false;
			}
			return key.as<int>() == 1;
		}

		void logError(std::string_view desc, const sol::error& err) noexcept
		{
			std::stringstream ss;
			ss << "recovered lua error " << desc << ":" << std::endl;
			ss << err.what() << std::endl;

			StreamUtils::logDebug(ss.str(), true);
		}

		bool checkResult(std::string_view desc, const sol::protected_function_result& result) noexcept
		{
			if (!result.valid())
			{
				logError(desc, result);
				return false;
			}
			sol::object obj = result;
			if (obj.get_type() == sol::type::boolean)
			{
				return obj.as<bool>();
			}
			return obj != sol::nil;
		}

		entt::id_type ptrTypeId(const void* ptr) noexcept
		{
			auto addr = reinterpret_cast<std::uintptr_t>(ptr);
			auto hash = std::hash<std::uintptr_t>{}(addr);
			return static_cast<entt::id_type>(hash);
		}

		std::optional<entt::id_type> getTypeId(const sol::object& type) noexcept
		{
			auto luaType = type.get_type();
			if (luaType == sol::type::number)
			{
				return type.template as<entt::id_type>();
			}
			if (type.is<sol::table>())
			{
				auto table = type.as<sol::table>();
				auto cls = table["class"];
				if (cls.get_type() == sol::type::table)
				{
					table = cls;
				}
				auto entry = table["type_id"];
				auto entryType = entry.get_type();
				if (entryType == sol::type::number)
				{
					return entry.get<entt::id_type>();
				}
				if (entry.is<sol::reference>())
				{
					return ptrTypeId(entry.get<sol::reference>().pointer());
				}
				return ptrTypeId(table.pointer());
			}
			return std::nullopt;
		}

		int LuaUtils::deny(lua_State* L)
		{
			return luaL_error(L, "operation not allowed");
		}

		void configProtobufField(sol::metatable& table, const google::protobuf::FieldDescriptor& field)
		{
			using namespace google::protobuf;
			auto fieldName = field.name();
			auto getField = [fieldName](const Message& msg)
			{
				return msg.GetDescriptor()->FindFieldByName(fieldName);
			};

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
					table[fieldName] = sol::property([getField](const Message& msg) {
						return msg.GetReflection()->GetInt32(msg, getField(msg));
						}, [getField](Message& msg, int32_t val) {
							msg.GetReflection()->SetInt32(&msg, getField(msg), val);
							});
						break;
				case FieldDescriptor::CPPTYPE_INT64:
					table[fieldName] = sol::property([getField](const Message& msg) {
						return msg.GetReflection()->GetInt64(msg, getField(msg));
						}, [getField](Message& msg, int64_t val) {
							msg.GetReflection()->SetInt64(&msg, getField(msg), val);
							});
						break;
				case FieldDescriptor::CPPTYPE_UINT32:
					table[fieldName] = sol::property([getField](const Message& msg) {
						return msg.GetReflection()->GetUInt32(msg, getField(msg));
						}, [getField](Message& msg, uint32_t val) {
							msg.GetReflection()->SetUInt32(&msg, getField(msg), val);
							});
						break;
				case FieldDescriptor::CPPTYPE_UINT64:
					table[fieldName] = sol::property([getField](const Message& msg) {
						return msg.GetReflection()->GetUInt64(msg, getField(msg));
						}, [getField](Message& msg, uint32_t val) {
							msg.GetReflection()->SetUInt64(&msg, getField(msg), val);
							});
						break;
				case FieldDescriptor::CPPTYPE_DOUBLE:
					table[fieldName] = sol::property([getField](const Message& msg) {
						return msg.GetReflection()->GetDouble(msg, getField(msg));
						}, [getField](Message& msg, double val) {
							msg.GetReflection()->SetDouble(&msg, getField(msg), val);
							});
						break;
				case FieldDescriptor::CPPTYPE_FLOAT:
					table[fieldName] = sol::property([getField](const Message& msg) {
						return msg.GetReflection()->GetFloat(msg, getField(msg));
						}, [getField](Message& msg, float val) {
							msg.GetReflection()->SetFloat(&msg, getField(msg), val);
							});
						break;
				case FieldDescriptor::CPPTYPE_BOOL:
					table[fieldName] = sol::property([getField](const Message& msg) {
						return msg.GetReflection()->GetBool(msg, getField(msg));
						}, [getField](Message& msg, bool val) {
							msg.GetReflection()->SetBool(&msg, getField(msg), val);
							});
						break;
				case FieldDescriptor::CPPTYPE_ENUM:
					table[fieldName] = sol::property([getField](const Message& msg) {
						return msg.GetReflection()->GetEnum(msg, getField(msg));
						}, [getField](Message& msg, int val) {
							auto field = getField(msg);
							auto enumVal = field->enum_type()->FindValueByNumber(val);
							msg.GetReflection()->SetEnum(&msg, field, enumVal);
							});
						break;
				case FieldDescriptor::CPPTYPE_STRING:
					table[fieldName] = sol::property([getField](const Message& msg) {
						return msg.GetReflection()->GetString(msg, getField(msg));
						}, [getField](Message& msg, const std::string& val) {
							msg.GetReflection()->SetString(&msg, getField(msg), val);
							});
						break;
				case FieldDescriptor::CPPTYPE_MESSAGE:
					/*
					table[fieldName] = sol::property([getField](const Message& msg) {
						return msg.GetReflection()->GetMessage(msg, getField(msg));
					}, [getField](google::protobuf::Message& msg, const Message& val) {
						msg.GetReflection()->SetMessage(&msg, getField(msg), val);
					});
					*/
					break;
				}
			}
		}

		void configProtobuf(sol::metatable& table, const google::protobuf::Descriptor& desc)
		{
			for (int i = 0; i < desc.field_count(); ++i)
			{
				const auto* field = desc.field(i);
				if (!field)
				{
					continue;
				}
				configProtobufField(table, *field);
			}
		}
	}

	LuaTableDelegateDefinition::LuaTableDelegateDefinition(const std::string& key, const std::string& desc) noexcept
		: _key(key)
		, _desc(desc)
	{
	}

	LuaDelegate::LuaDelegate(const sol::object& obj, const std::string& tableKey) noexcept
		: _tableKey(tableKey.empty() ? "__call" : tableKey)
		, _obj(obj)
	{
	}

	LuaDelegate::operator bool() const noexcept
	{
		auto type = _obj.get_type();
		return type == sol::type::function || type == sol::type::table;
	}

	bool LuaDelegate::operator==(const sol::object& obj) const noexcept
	{
		return _obj == obj;
	}

	bool LuaDelegate::operator!=(const sol::object& obj) const noexcept
	{
		return !operator==(obj);
	}

	bool LuaDelegate::operator==(const LuaDelegate& dlg) const noexcept
	{
		return _obj == dlg._obj && _tableKey == dlg._tableKey;
	}

	bool LuaDelegate::operator!=(const LuaDelegate& dlg) const noexcept
	{
		return !operator==(dlg);
	}
}