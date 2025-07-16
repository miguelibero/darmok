#pragma once

#include <darmok/export.h>
#include <darmok/asset_core.hpp>
#include <darmok/expected.hpp>
#include <darmok/scene_fwd.hpp>

#include <memory>
#include <filesystem>
#include <string>

#include <bx/bx.h>
#include <bx/allocator.h>

namespace darmok
{
	class IDataLoader;
	class ISceneLoader;
	class IProgramLoader;
	class IMeshLoader;
	class ITextureLoader;
	class ITextureAtlasLoader;
	class IMaterialLoader;
	class IArmatureLoader;
	class IFontLoader;
	class ISkeletonLoader;
	class ISkeletalAnimationLoader;
	class ISkeletalAnimatorDefinitionLoader;
	class ISoundLoader;
	class IMusicLoader;
	class IImageLoader;

	class DARMOK_EXPORT BX_NO_VTABLE IAssetContext
	{
	public:
		virtual ~IAssetContext() = default;

		[[nodiscard]] virtual bx::AllocatorI& getAllocator() noexcept = 0;
		[[nodiscard]] virtual IProgramLoader& getProgramLoader() noexcept = 0;
		[[nodiscard]] virtual ITextureLoader& getTextureLoader() noexcept = 0;
		[[nodiscard]] virtual IMeshLoader& getMeshLoader() noexcept = 0;
		[[nodiscard]] virtual IMaterialLoader& getMaterialLoader() noexcept = 0;
		[[nodiscard]] virtual IArmatureLoader& getArmatureLoader() noexcept = 0;
		[[nodiscard]] virtual ITextureAtlasLoader& getTextureAtlasLoader() noexcept = 0;
		[[nodiscard]] virtual ISceneLoader& getSceneLoader() noexcept = 0;
		[[nodiscard]] virtual IFontLoader& getFontLoader() noexcept = 0;
		[[nodiscard]] virtual ISkeletonLoader& getSkeletonLoader() noexcept = 0;
		[[nodiscard]] virtual ISkeletalAnimationLoader& getSkeletalAnimationLoader() noexcept = 0;
		[[nodiscard]] virtual ISkeletalAnimatorDefinitionLoader& getSkeletalAnimatorDefinitionLoader() noexcept = 0;
		[[nodiscard]] virtual ISoundLoader& getSoundLoader() noexcept = 0;
		[[nodiscard]] virtual IMusicLoader& getMusicLoader() noexcept = 0;
	};

	class AssetContextImpl;

	struct AssetContextConfig final
	{
		IDataLoader& dataLoader;
		bx::AllocatorI& allocator;
	};

	class DARMOK_EXPORT AssetContext final : public IAssetContext
	{
	public:
		using Config = AssetContextConfig;
		AssetContext(Config&& config) noexcept;
		~AssetContext() noexcept;
		AssetContext(const AssetContext&) = delete;
		AssetContext(AssetContext&&) = delete;
		AssetContext& operator=(const AssetContext&) = delete;
		AssetContext& operator=(AssetContext&&) = delete;


		[[nodiscard]] IDataLoader& getDataLoader() noexcept;
		[[nodiscard]] IImageLoader& getImageLoader() noexcept;
		[[nodiscard]] bx::AllocatorI& getAllocator() noexcept override;
		[[nodiscard]] IProgramLoader& getProgramLoader() noexcept override;
		[[nodiscard]] ITextureLoader& getTextureLoader() noexcept override;
		[[nodiscard]] IMeshLoader& getMeshLoader() noexcept override;
		[[nodiscard]] IMaterialLoader& getMaterialLoader() noexcept override;
		[[nodiscard]] IArmatureLoader& getArmatureLoader() noexcept override;
		[[nodiscard]] ITextureAtlasLoader& getTextureAtlasLoader() noexcept override;
		[[nodiscard]] ISceneLoader& getSceneLoader() noexcept override;
		[[nodiscard]] IFontLoader& getFontLoader() noexcept override;
		[[nodiscard]] ISkeletonLoader& getSkeletonLoader() noexcept override;
		[[nodiscard]] ISkeletalAnimationLoader& getSkeletalAnimationLoader() noexcept override;
		[[nodiscard]] ISkeletalAnimatorDefinitionLoader& getSkeletalAnimatorDefinitionLoader() noexcept override;
		[[nodiscard]] ISoundLoader& getSoundLoader() noexcept override;
		[[nodiscard]] IMusicLoader& getMusicLoader() noexcept override;

		[[nodiscard]] AssetContextImpl& getImpl() noexcept;
		[[nodiscard]] const AssetContextImpl& getImpl() const noexcept;

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