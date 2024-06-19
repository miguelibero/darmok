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

	class DARMOK_EXPORT BX_NO_VTABLE IOzzRawSkeletonLoader
	{
	public:
		virtual ~IOzzRawSkeletonLoader() = default;
		using result_type = ozz::animation::offline::RawSkeleton;
		virtual result_type operator()(std::string_view name) = 0;
	};

	class OzzSkeletonProcessorImpl;

	class DARMOK_EXPORT OzzSkeletonProcessor final : public IAssetTypeProcessor
	{
	public:
		OzzSkeletonProcessor(std::unique_ptr<IOzzRawSkeletonLoader>&& loader) noexcept;
		~OzzSkeletonProcessor() noexcept;
		bool getOutputs(const std::filesystem::path& input, std::vector<std::filesystem::path>& outputs) override;
		std::ofstream createOutputStream(const std::filesystem::path& input, size_t outputIndex, const std::filesystem::path& path) override;
		void writeOutput(const std::filesystem::path& input, size_t outputIndex, std::ostream& out) override;
		std::string getName() const noexcept override;
	private:
		std::unique_ptr<OzzSkeletonProcessorImpl> _impl;
	};
}