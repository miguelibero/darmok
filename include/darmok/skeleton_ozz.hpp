#pragma once

#include <darmok/export.h>
#include <darmok/skeleton.hpp>
#include <darmok/asset_core.hpp>
#include <string>

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
		std::shared_ptr<Skeleton> operator()(std::string_view name) override;
	private:
		IDataLoader& _dataLoader;
	};

	class DARMOK_EXPORT OzzSkeletalAnimationLoader final : public ISkeletalAnimationLoader
	{
	public:
		OzzSkeletalAnimationLoader(IDataLoader& dataLoader) noexcept;
		std::shared_ptr<SkeletalAnimation> operator()(std::string_view name) override;
	private:
		IDataLoader& _dataLoader;
	};

	class DARMOK_EXPORT OzzFbxSkeletonProcessor final : public IAssetTypeProcessor
	{
	public:
		OzzFbxSkeletonProcessor() noexcept;
		ozz::animation::offline::RawSkeleton read(const std::filesystem::path& path) const;
		bool getOutputs(const std::filesystem::path& input, std::vector<std::filesystem::path>& outputs) const override;
		std::ofstream createOutputStream(size_t outputIndex, const std::filesystem::path& path) const override;
		void writeOutput(const std::filesystem::path& input, size_t outputIndex, std::ostream& out) const override;
		std::string getName() const noexcept override;
	};

}