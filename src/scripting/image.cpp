#include "image.hpp"
#include <darmok/image.hpp>

namespace darmok
{
    LuaImage::LuaImage(const std::shared_ptr<Image>& img) noexcept
		: _img(img)
	{
	}

	std::shared_ptr<Image> LuaImage::getReal() const noexcept
	{
		return _img;
	}

	bool LuaImage::empty() const noexcept
	{
		return _img == nullptr || _img->empty();
	}

	glm::uvec2 LuaImage::getSize() const noexcept
	{
		if (_img == nullptr)
		{
			return glm::uvec2(0);
		}
		return _img->getSize();
	}

	uint32_t LuaImage::getDepth() const noexcept
	{
		if (_img == nullptr)
		{
			return 0;
		}
		return _img->getDepth();
	}
	bool LuaImage::isCubeMap() const noexcept
	{
		if (_img == nullptr)
		{
			return false;
		}
		return _img->isCubeMap();
	}

	uint8_t LuaImage::getMipCount() const noexcept
	{
		if (_img == nullptr)
		{
			return 0;
		}
		return _img->getMipCount();
	}

	uint16_t LuaImage::getLayerCount() const noexcept
	{
		if (_img == nullptr)
		{
			return 0;
		}
		return _img->getLayerCount();
	}

	void LuaImage::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaImage>("Image",
			sol::constructors<>(),
			"size", sol::property(&LuaImage::getSize),
			"depth", sol::property(&LuaImage::getDepth),
			"is_cubemap", sol::property(&LuaImage::isCubeMap),
			"mip_count", sol::property(&LuaImage::getMipCount),
			"layer_count", sol::property(&LuaImage::getLayerCount)
		);
	}
}