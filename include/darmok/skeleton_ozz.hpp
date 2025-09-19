#pragma once

#include <darmok/export.h>
#include <darmok/skeleton.hpp>
#include <darmok/asset_core.hpp>
#include <string>
#include <bx/bx.h>

namespace ozz::animation::offline
{
	struct RawSkeleton;
}

namespace darmok
{
    class IDataLoader;

    class DARMOK_EXPORT OzzSkeletonLoader final : public ISkeletonLoader
	{
	public:
		OzzSkeletonLoader(IDataLoader& dataLoader) noexcept;
		Result operator()(std::filesystem::path path) noexcept override;
	private:
		IDataLoader& _dataLoader;
	};

	class DARMOK_EXPORT OzzSkeletalAnimationLoader final : public ISkeletalAnimationLoader
	{
	public:
		OzzSkeletalAnimationLoader(IDataLoader& dataLoader) noexcept;
		Result operator()(std::filesystem::path path) noexcept override;
	private:
		IDataLoader& _dataLoader;
	};
}