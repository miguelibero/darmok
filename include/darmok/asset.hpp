#pragma once

#include <darmok/export.h>
#include <darmok/asset_core.hpp>
#include <bx/allocator.h>
#include <memory>
#include <filesystem>

namespace darmok
{
	class App;
	class AssetContextImpl;
	class IDataLoader;
	class IImageLoader;
	class IProgramLoader;
	class ITextureLoader;
	class ITextureAtlasLoader;
    class AssetContext;
	class IModelLoader;

#ifdef DARMOK_ASSIMP
	class AssimpModelLoader;
#endif

#ifdef DARMOK_OZZ
	class ISkeletonLoader;
	class ISkeletalAnimationLoader;
	class ISkeletalAnimatorConfigLoader;
#endif

#ifdef DARMOK_FREETYPE
	class IFontLoader;
#endif

#ifdef DARMOK_MINIAUDIO
	class ISoundLoader;
	class IMusicLoader;
#endif

	class DARMOK_EXPORT AssetContext final
	{
	public:
		AssetContext() noexcept;
		~AssetContext() noexcept;

		[[nodiscard]] IDataLoader& getDataLoader() noexcept;
		[[nodiscard]] IImageLoader& getImageLoader() noexcept;
		[[nodiscard]] IProgramLoader& getProgramLoader() noexcept;
		[[nodiscard]] ITextureLoader& getTextureLoader() noexcept;
		[[nodiscard]] ITextureAtlasLoader& getTextureAtlasLoader() noexcept;
		[[nodiscard]] IModelLoader& getModelLoader() noexcept;
		[[nodiscard]] bx::AllocatorI& getAllocator() noexcept;

#ifdef DARMOK_ASSIMP
		[[nodiscard]] AssimpModelLoader& getAssimpModelLoader() noexcept;
#endif

#ifdef DARMOK_OZZ
		[[nodiscard]] ISkeletonLoader& getSkeletonLoader() noexcept;
		[[nodiscard]] ISkeletalAnimationLoader& getSkeletalAnimationLoader() noexcept;
		[[nodiscard]] ISkeletalAnimatorConfigLoader& getSkeletalAnimatorConfigLoader() noexcept;
#endif

#ifdef DARMOK_FREETYPE
		[[nodiscard]] IFontLoader& getFontLoader() noexcept;
#endif

#ifdef DARMOK_MINIAUDIO
		[[nodiscard]] ISoundLoader& getSoundLoader() noexcept;
		[[nodiscard]] IMusicLoader& getMusicLoader() noexcept;
#endif

		[[nodiscard]] AssetContextImpl& getImpl() noexcept;
		[[nodiscard]] const AssetContextImpl& getImpl() const noexcept;

		AssetContext& addBasePath(const std::filesystem::path& path) noexcept;
		bool removeBasePath(const std::filesystem::path& path) noexcept;

	private:
		std::unique_ptr<AssetContextImpl> _impl;
	};

	class ProgramImporter;

	class DARMOK_EXPORT DarmokAssetImporter final
	{
	public:
		DarmokAssetImporter(const CommandLineAssetImporterConfig& config);
		DarmokAssetImporter(const std::filesystem::path& inputPath);
		DarmokAssetImporter& setCachePath(const std::filesystem::path& cachePath) noexcept;
		DarmokAssetImporter& setOutputPath(const std::filesystem::path& outputPath) noexcept;
		DarmokAssetImporter& setShadercPath(const std::filesystem::path& path) noexcept;
		DarmokAssetImporter& addShaderIncludePath(const std::filesystem::path& path) noexcept;
		std::vector<std::filesystem::path> getOutputs() const;
		void operator()(std::ostream& log) const;
	private:
		AssetImporter _importer;
		ProgramImporter& _progImporter;
	};
}