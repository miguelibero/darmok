#pragma once

#include <bx/bx.h>
#include <vector>
#include <string_view>

namespace darmok
{
    class BX_NO_VTABLE IAssetFinder
	{
	public:
		virtual ~IAssetFinder() = default;
		virtual std::vector<std::string> operator()(std::string_view pattern) = 0;
	};
}