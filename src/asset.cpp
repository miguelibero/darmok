#include "asset.hpp"

#include <filesystem>
#include <bx/filepath.h>

#ifdef DARMOK_ASSIMP
#ifdef DARMOK_OZZ
#include <darmok/skeleton_assimp.hpp>
#endif
#endif

namespace darmok
{
	static std::filesystem::path fixAssetFilePath(const bx::FilePath& filePath, const std::filesystem::path& basePath) noexcept
	{
		return basePath / std::filesystem::path(filePath.getCPtr());
	}

	void FileReader::addBasePath(const std::filesystem::path& basePath) noexcept
	{
		_basePaths.insert(_basePaths.begin(), basePath);
	}

	bool FileReader::removeBasePath(const std::filesystem::path& path) noexcept
	{
		auto itr = std::remove(_basePaths.begin(), _basePaths.end(), path);
		if (itr == _basePaths.end())
		{
			return false;
		}
		_basePaths.erase(itr, _basePaths.end());
		return true;
	}

	bool FileReader::open(const bx::FilePath& filePath, bx::Error* err)
	{
		for (auto& basePath : _basePaths)
		{
			auto path = fixAssetFilePath(filePath, basePath);
			if (std::filesystem::exists(path))
			{
				return bx::FileReader::open(path.string().c_str(), err);
			}
		}
		return false;
	}

	AssetContextImpl::AssetContextImpl()
		: _dataLoader(_fileReader, _allocator)
		, _imageLoader(_dataLoader, _allocator)
		, _programLoader(_dataLoader)
		, _textureLoader(_imageLoader)
		, _textureAtlasLoader(_dataLoader, _textureLoader)
		, _binModelLoader(_dataLoader)
		, _textureAtlasFontLoader(_textureAtlasLoader)
#ifdef DARMOK_OZZ
		, _skeletalAnimatorConfigLoader(_dataLoader)
		, _ozzSkeletonLoader(_dataLoader)
		, _ozzSkeletalAnimationLoader(_dataLoader)
#endif		
#ifdef DARMOK_ASSIMP
		, _assimpModelLoader(_dataLoader, _allocator, _imageLoader)
#endif		
#ifdef DARMOK_FREETYPE
		, _freetypeFontLoader(_dataLoader, _allocator)
#endif
#ifdef DARMOK_MINIAUDIO
		, _miniaudioSoundLoader(_dataLoader)
		, _miniaudioMusicLoader(_dataLoader)
#endif
	{
		addBasePath("assets");

#ifdef DARMOK_OZZ
		_skeletonLoader.setDefaultLoader(_ozzSkeletonLoader);
		_skeletalAnimationLoader.setDefaultLoader(_ozzSkeletalAnimationLoader);
#endif

		_modelLoader.setDefaultLoader(_binModelLoader);
		_modelLoader.addLoader(".dml;.bin", _binModelLoader);

#ifdef DARMOK_ASSIMP
		_modelLoader.setDefaultLoader(_assimpModelLoader);
#endif

		_fontLoader.setDefaultLoader(_textureAtlasFontLoader);
		_fontLoader.addLoader(".xml", _textureAtlasFontLoader);
#ifdef DARMOK_FREETYPE
		_fontLoader.setDefaultLoader(_freetypeFontLoader);
#endif
	}

	bx::AllocatorI& AssetContextImpl::getAllocator() noexcept
	{
		return _allocator;
	}

	[[nodiscard]] IDataLoader& AssetContextImpl::getDataLoader() noexcept
	{
		return _dataLoader;
	}
	
	IImageLoader& AssetContextImpl::getImageLoader() noexcept
	{
		return _imageLoader;
	}

	IProgramLoader& AssetContextImpl::getProgramLoader() noexcept
	{
		return _programLoader;
	}

	ITextureLoader& AssetContextImpl::getTextureLoader() noexcept
	{
		return _textureLoader;
	}

	ITextureAtlasLoader& AssetContextImpl::getTextureAtlasLoader() noexcept
	{
		return _textureAtlasLoader;
	}

	IModelLoader& AssetContextImpl::getModelLoader() noexcept
	{
		return _modelLoader;
	}

#ifdef DARMOK_ASSIMP
	AssimpModelLoader& AssetContextImpl::getAssimpModelLoader() noexcept
	{
		return _assimpModelLoader;
	}
#endif
	
#ifdef DARMOK_OZZ
	ISkeletonLoader& AssetContextImpl::getSkeletonLoader() noexcept
	{
		return _skeletonLoader;
	}

	ISkeletalAnimationLoader& AssetContextImpl::getSkeletalAnimationLoader() noexcept
	{
		return _skeletalAnimationLoader;
	}

	ISkeletalAnimatorConfigLoader& AssetContextImpl::getSkeletalAnimatorConfigLoader() noexcept
	{
		return _skeletalAnimatorConfigLoader;
	}
#endif

#ifdef DARMOK_FREETYPE
	IFontLoader& AssetContextImpl::getFontLoader() noexcept
	{
		return _fontLoader;
	}
#endif

#ifdef DARMOK_MINIAUDIO
	ISoundLoader& AssetContextImpl::getSoundLoader() noexcept
	{
		return _miniaudioSoundLoader;
	}

	IMusicLoader& AssetContextImpl::getMusicLoader() noexcept
	{
		return _miniaudioMusicLoader;
	}
#endif

	void AssetContextImpl::addBasePath(const std::filesystem::path& path) noexcept
	{
		_fileReader.addBasePath(path);
	}

	bool AssetContextImpl::removeBasePath(const std::filesystem::path& path) noexcept
	{
		return _fileReader.removeBasePath(path);
	}

	void AssetContextImpl::init(App& app)
	{
		_freetypeFontLoader.init(app);
	}

	void AssetContextImpl::update()
	{
	}

	void AssetContextImpl::shutdown()
	{
		_freetypeFontLoader.shutdown();
	}

	AssetContext::AssetContext() noexcept
		: _impl(std::make_unique<AssetContextImpl>())
	{
	}

	AssetContext::~AssetContext() noexcept
	{
	}

	IDataLoader& AssetContext::getDataLoader() noexcept
	{
		return _impl->getDataLoader();
	}

	IImageLoader& AssetContext::getImageLoader() noexcept
	{
		return _impl->getImageLoader();
	}

	IProgramLoader& AssetContext::getProgramLoader() noexcept
	{
		return _impl->getProgramLoader();
	}

	ITextureLoader& AssetContext::getTextureLoader() noexcept
	{
		return _impl->getTextureLoader();
	}

	ITextureAtlasLoader& AssetContext::getTextureAtlasLoader() noexcept
	{
		return _impl->getTextureAtlasLoader();
	}

	IModelLoader& AssetContext::getModelLoader() noexcept
	{
		return _impl->getModelLoader();
	}

#ifdef DARMOK_ASSIMP
	AssimpModelLoader& AssetContext::getAssimpModelLoader() noexcept
	{
		return _impl->getAssimpModelLoader();
	}
#endif

#ifdef DARMOK_OZZ
	ISkeletonLoader& AssetContext::getSkeletonLoader() noexcept
	{
		return _impl->getSkeletonLoader();
	}

	ISkeletalAnimationLoader& AssetContext::getSkeletalAnimationLoader() noexcept
	{
		return _impl->getSkeletalAnimationLoader();
	}

	ISkeletalAnimatorConfigLoader& AssetContext::getSkeletalAnimatorConfigLoader() noexcept
	{
		return _impl->getSkeletalAnimatorConfigLoader();
	}
#endif

#ifdef DARMOK_FREETYPE
	IFontLoader& AssetContext::getFontLoader() noexcept
	{
		return _impl->getFontLoader();
	}
#endif

#ifdef DARMOK_MINIAUDIO
	ISoundLoader& AssetContext::getSoundLoader() noexcept
	{
		return _impl->getSoundLoader();
	}

	IMusicLoader& AssetContext::getMusicLoader() noexcept
	{
		return _impl->getMusicLoader();
	}
#endif

	bx::AllocatorI& AssetContext::getAllocator() noexcept
	{
		return _impl->getAllocator();
	}

	AssetContextImpl& AssetContext::getImpl() noexcept
	{
		return *_impl;
	}

	const AssetContextImpl& AssetContext::getImpl() const noexcept
	{
		return *_impl;
	}

	AssetContext& AssetContext::addBasePath(const std::filesystem::path& path) noexcept
	{
		_impl->addBasePath(path);
		return *this;
	}

	bool AssetContext::removeBasePath(const std::filesystem::path& path) noexcept
	{
		return _impl->removeBasePath(path);
	}

	void AssetContext::init(App& app)
	{
		_impl->init(app);
	}

	void AssetContext::update()
	{
		_impl->update();
	}

	void AssetContext::shutdown()
	{
		_impl->shutdown();
	}

	DarmokAssetImporter::DarmokAssetImporter(const CommandLineAssetImporterConfig& config)
		: DarmokAssetImporter(config.inputPath)
	{
		if (!config.cachePath.empty())
		{
			setCachePath(config.cachePath);
		}
		if (!config.outputPath.empty())
		{
			setOutputPath(config.outputPath);
		}
		if (!config.shadercPath.empty())
		{
			setShadercPath(config.shadercPath);
		}
		for (auto& path : config.shaderIncludePaths)
		{
			addShaderIncludePath(path);
		}
	}

	DarmokAssetImporter::DarmokAssetImporter(const std::filesystem::path& inputPath)
		: _importer(inputPath)
		, _progImporter(_importer.addTypeImporter<ProgramImporter>())
	{
#ifdef DARMOK_ASSIMP
		_importer.addTypeImporter<AssimpModelImporter>();
#ifdef DARMOK_OZZ
		_importer.addTypeImporter<SkeletalAnimatorConfigImporter>();
		_importer.addTypeImporter<AssimpSkeletonImporter>();
		_importer.addTypeImporter<AssimpSkeletalAnimationImporter>();
#endif
#endif
#ifdef DARMOK_FREETYPE
		_importer.addTypeImporter<FreetypeFontAtlasImporter>();
#endif
		_importer.addTypeImporter<CopyAssetImporter>();
		_importer.addTypeImporter<TexturePackerTextureAtlasImporter>();
		_importer.addTypeImporter<ImageImporter>();
	}

	DarmokAssetImporter& DarmokAssetImporter::setCachePath(const std::filesystem::path& cachePath) noexcept
	{
		_importer.setCachePath(cachePath);
		return *this;
	}

	DarmokAssetImporter& DarmokAssetImporter::setOutputPath(const std::filesystem::path& outputPath) noexcept
	{
		_importer.setOutputPath(outputPath);
		return *this;
	}

	DarmokAssetImporter& DarmokAssetImporter::setShadercPath(const std::filesystem::path& path) noexcept
	{
		_progImporter.setShadercPath(path);
		return *this;
	}

	DarmokAssetImporter& DarmokAssetImporter::addShaderIncludePath(const std::filesystem::path& path) noexcept
	{
		_progImporter.addIncludePath(path);
		return *this;
	}

	std::vector<std::filesystem::path> DarmokAssetImporter::getOutputs() const
	{
		return _importer.getOutputs();
	}

	void DarmokAssetImporter::operator()(std::ostream& log) const
	{
		return _importer(log);
	}
}
