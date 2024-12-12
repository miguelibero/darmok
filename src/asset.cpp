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
	
	AssetContextImpl::AssetContextImpl()
		: _dataLoader(_allocator)
		, _imageLoader(_dataLoader, _allocator)
		, _programDefLoader(_dataLoader)
		, _cerealTexDefLoader(_dataLoader)
		, _imgTexDefLoader(_imageLoader)
		, _texDefLoader{ _imgTexDefLoader, _cerealTexDefLoader }
		, _meshDefLoader(_dataLoader)
		, _programLoader(_programDefLoader)
		, _textureLoader(_texDefLoader)
		, _meshLoader(_meshDefLoader)
		, _texPackerDefLoader(_dataLoader, _texDefLoader)
		, _textureAtlasLoader({ _texPackerDefLoader }, _textureLoader)
		, _binModelLoader(_dataLoader)
		, _textureAtlasFontLoader(_textureAtlasLoader)
#ifdef DARMOK_OZZ
		, _skeletalAnimatorConfigLoader(_dataLoader)
		, _ozzSkeletonLoader(_dataLoader)
		, _ozzSkeletalAnimationLoader(_dataLoader)
		, _skeletonLoader(_ozzSkeletonLoader)
		, _skeletalAnimationLoader(_ozzSkeletalAnimationLoader)
#endif		
#ifdef DARMOK_ASSIMP
		, _assimpModelLoader(_dataLoader, _allocator, _imageLoader)
		, _modelLoader(_assimpModelLoader)
#else
		, _modelLoader(_binModelLoader)
#endif		
#ifdef DARMOK_FREETYPE
		, _freetypeFontLoader(_dataLoader, _allocator)
		, _fontLoader(_freetypeFontLoader)
#else
		, _fontLoader(_textureAtlasFontLoader);
#endif
#ifdef DARMOK_MINIAUDIO
		, _miniaudioSoundLoader(_dataLoader)
		, _miniaudioMusicLoader(_dataLoader)
#endif
	{
		addBasePath("assets");
		_modelLoader.addLoader(".dml;.bin", _binModelLoader);
		_fontLoader.addLoader(".xml", _textureAtlasFontLoader);
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

	IMeshLoader& AssetContextImpl::getMeshLoader() noexcept
	{
		return _meshLoader;
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
		_dataLoader.addBasePath(path);
	}

	bool AssetContextImpl::removeBasePath(const std::filesystem::path& path) noexcept
	{
		return _dataLoader.removeBasePath(path);
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

	IMeshLoader& AssetContext::getMeshLoader() noexcept
	{
		return _impl->getMeshLoader();
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
		_importer.addTypeImporter<AssimpModelImporter>(_alloc);
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
