#pragma once

#include <memory>
#include <bx/allocator.h>

namespace darmok
{
	class AssetContextImpl;
	class IDataLoader;
	class IImageLoader;
	class IProgramLoader;
	class StandardProgramLoader;
	class ITextureLoader;
	class ITextureAtlasLoader;
	class ColorTextureLoader;
    class AssetContext;
	class ISkeletonLoader;
	class ISkeletalAnimationLoader;
	class ISkeletalAnimatorConfigLoader;
	class IModelLoader;

#ifdef DARMOK_ASSIMP
	class AssimpModelLoader;
#endif

	class AssetContext final
	{
	public:
		AssetContext() noexcept;
		~AssetContext() noexcept;

		[[nodiscard]] DLLEXPORT IDataLoader& getDataLoader() noexcept;
		[[nodiscard]] DLLEXPORT IImageLoader& getImageLoader() noexcept;
		[[nodiscard]] DLLEXPORT IProgramLoader& getProgramLoader() noexcept;
		[[nodiscard]] DLLEXPORT StandardProgramLoader& getStandardProgramLoader() noexcept;
		[[nodiscard]] DLLEXPORT ITextureLoader& getTextureLoader() noexcept;
		[[nodiscard]] DLLEXPORT ITextureAtlasLoader& getTextureAtlasLoader() noexcept;
		[[nodiscard]] DLLEXPORT ColorTextureLoader& getColorTextureLoader() noexcept;
		[[nodiscard]] DLLEXPORT ISkeletonLoader& getSkeletonLoader() noexcept;
		[[nodiscard]] DLLEXPORT ISkeletalAnimationLoader& getSkeletalAnimationLoader() noexcept;
		[[nodiscard]] DLLEXPORT ISkeletalAnimatorConfigLoader& getSkeletalAnimatorConfigLoader() noexcept;
		[[nodiscard]] DLLEXPORT IModelLoader& getModelLoader() noexcept;
		[[nodiscard]] DLLEXPORT bx::AllocatorI& getAllocator() noexcept;

#ifdef DARMOK_ASSIMP
		[[nodiscard]] DLLEXPORT AssimpModelLoader& getAssimpModelLoader() noexcept;
#endif

		[[nodiscard]] AssetContextImpl& getImpl() noexcept;
		[[nodiscard]] const AssetContextImpl& getImpl() const noexcept;
	private:
		std::unique_ptr<AssetContextImpl> _impl;
	};
}