#pragma once

#include <bx/bx.h>
#include <string_view>

namespace darmok
{
    class ISkeleton
    {

    };

	class ISkeletalAnimation
    {

    };

    class BX_NO_VTABLE ISkeletonLoader
	{
	public:
		virtual ~ISkeletonLoader() = default;
		virtual std::shared_ptr<ISkeleton> operator()(std::string_view name) = 0;
	};
}