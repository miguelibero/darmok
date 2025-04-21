#pragma once

#include <darmok/export.h>
#include <darmok/asset_core.hpp>

#include <memory>
#include <filesystem>

#include <bx/allocator.h>

namespace darmok
{
	class App;
	class AssetContextImpl;

	class IDataLoader;
	class IImageLoader;
	class IProgramLoader;
	class IMeshLoader;
	class ITextureLoader;
	class ITextureAtlasLoader;
	class IMaterialLoader;
	class IFontLoader;
	class ISceneLoader;

#ifdef DARMOK_OZZ
	class ISkeletonLoader;
	class ISkeletalAnimationLoader;
	class ISkeletalAnimatorDefinitionLoader;
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
		AssetContext(const AssetContext&) = delete;
		AssetContext(AssetContext&&) = delete;
		AssetContext& operator=(const AssetContext&) = delete;
		AssetContext& operator=(AssetContext&&) = delete;

		[[nodiscard]] IDataLoader& getDataLoader() noexcept;
		[[nodiscard]] IImageLoader& getImageLoader() noexcept;
		[[nodiscard]] IProgramLoader& getProgramLoader() noexcept;
		[[nodiscard]] ITextureLoader& getTextureLoader() noexcept;
		[[nodiscard]] IMeshLoader& getMeshLoader() noexcept;
		[[nodiscard]] IMaterialLoader& getMaterialLoader() noexcept;
		[[nodiscard]] ITextureAtlasLoader& getTextureAtlasLoader() noexcept;
		[[nodiscard]] ISceneLoader& getSceneLoader() noexcept;
		[[nodiscard]] IFontLoader& getFontLoader() noexcept;
		[[nodiscard]] bx::AllocatorI& getAllocator() noexcept;

#ifdef DARMOK_OZZ
		[[nodiscard]] ISkeletonLoader& getSkeletonLoader() noexcept;
		[[nodiscard]] ISkeletalAnimationLoader& getSkeletalAnimationLoader() noexcept;
		[[nodiscard]] ISkeletalAnimatorDefinitionLoader& getSkeletalAnimatorDefinitionLoader() noexcept;
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

	class ProgramFileImporter;

	class DARMOK_EXPORT DarmokAssetFileImporter final
	{
	public:
		DarmokAssetFileImporter(const CommandLineFileImporterConfig& config);
		DarmokAssetFileImporter(const std::filesystem::path& inputPath);
		DarmokAssetFileImporter& setCachePath(const std::filesystem::path& cachePath) noexcept;
		DarmokAssetFileImporter& setOutputPath(const std::filesystem::path& outputPath) noexcept;
		DarmokAssetFileImporter& setShadercPath(const std::filesystem::path& path) noexcept;
		DarmokAssetFileImporter& addShaderIncludePath(const std::filesystem::path& path) noexcept;
		std::vector<std::filesystem::path> getOutputs() const;
		void operator()(std::ostream& log) const;
	private:
		bx::DefaultAllocator _alloc;
		FileImporter _importer;
		ProgramFileImporter& _progImporter;
	};
}