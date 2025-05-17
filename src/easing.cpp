#include <darmok/easing.hpp>

#include <magic_enum/magic_enum.hpp>
#include <algorithm>


namespace darmok
{
    std::string_view Easing::getTypeName(Type type) noexcept
    {
		return magic_enum::enum_name(type);
    }

    std::optional<Easing::Type> Easing::readType(std::string_view name) noexcept
    {
		return magic_enum::enum_cast<Easing::Type>(name);
    }
}