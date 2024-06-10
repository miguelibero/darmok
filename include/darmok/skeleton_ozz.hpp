#pragma once

#include <darmok/skeleton.hpp>

namespace darmok
{
    class IDataLoader;

    class OzzSkeletonLoader final : public ISkeletonLoader
	{
	public:
		OzzSkeletonLoader(IDataLoader& dataLoader) noexcept;
		std::shared_ptr<Skeleton> operator()(std::string_view name) override;
	private:
		IDataLoader& _dataLoader;
	};

	class OzzSkeletalAnimationLoader final : public ISkeletalAnimationLoader
	{
	public:
		OzzSkeletalAnimationLoader(IDataLoader& dataLoader) noexcept;
		std::shared_ptr<SkeletalAnimation> operator()(std::string_view name) override;
	private:
		IDataLoader& _dataLoader;
	};

}