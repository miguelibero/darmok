#include "image.hpp"
#include <darmok/image.hpp>

namespace darmok
{
	void LuaImage::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<Image>("Image", sol::no_constructor,
			"size", sol::property(&Image::getSize),
			"depth", sol::property(&Image::getDepth),
			"is_cubemap", sol::property(&Image::isCubeMap),
			"mip_count", sol::property(&Image::getMipCount),
			"layer_count", sol::property(&Image::getLayerCount)
		);
	}
}