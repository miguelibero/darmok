#include "lua/scene_filter.hpp"
#include "lua/utils.hpp"
#include <darmok/scene_filter.hpp>

namespace darmok
{

	EntityFilter LuaEntityFilter::operatorOr(const EntityFilter& filter, const sol::object& type) noexcept
	{
		if (type.is<EntityFilter>())
		{
			return filter | type.as<EntityFilter>();
		}
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			return filter | typeId.value();
		}
		return filter;
	}

	EntityFilter LuaEntityFilter::operatorAnd(const EntityFilter& filter, const sol::object& type) noexcept
	{
		if (type.is<EntityFilter>())
		{
			return filter & type.as<EntityFilter>();
		}
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			return filter & typeId.value();
		}
		return EntityFilter({}, EntityFilterOperation::And);
	}

	EntityFilter LuaEntityFilter::operatorNot(const EntityFilter& filter) noexcept
	{
		return EntityFilter({ filter }, EntityFilterOperation::Not);
	}

	EntityFilter LuaEntityFilter::create(const sol::object& value) noexcept
	{
		if (value.is<EntityFilter>())
		{
			return value.as<EntityFilter>();
		}
		auto type = value.get_type();
		if (type == sol::type::nil)
		{
			return EntityFilter();
		}
		if (type == sol::type::table)
		{
			sol::table tab = value;
			if (LuaUtils::isArray(tab))
			{
				EntityFilter filter;
				for (auto [key, val] : tab)
				{
					filter |= create(val);
				}
				return filter;
			}
		}
		if (auto typeId = LuaUtils::getTypeId(value))
		{
			return EntityFilter(typeId.value());
		}
		return {};
	}

	void LuaEntityFilter::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<EntityFilter>("EntityFilter",
			sol::factories(&LuaEntityFilter::create),
			sol::call_constructor, &LuaEntityFilter::create,
			sol::meta_function::bitwise_and, &LuaEntityFilter::operatorAnd,
			sol::meta_function::bitwise_or, &LuaEntityFilter::operatorOr,
			sol::meta_function::bitwise_not, &LuaEntityFilter::operatorNot,
			sol::meta_function::to_string, &EntityFilter::toString
		);
	}
}