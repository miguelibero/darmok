#include "detail/asset.hpp"

#include <darmok/asset_pack.hpp>


namespace darmok
{

	AssetContextImpl::AssetContextImpl(IAssetContext& assets, Config&& config)
		: _config{ std::move(config) }
		, _imageLoader{ getDataLoader(), getAllocator() }
		, _dataProgDefLoader{ getDataLoader() }
		, _dataTexDefLoader{ getDataLoader() }
		, _imgTexDefLoader{ _imageLoader }
		, _texDefLoader{ _imgTexDefLoader, _dataTexDefLoader }
		, _dataMeshDefLoader{ getDataLoader() }
		, _progLoader{ _dataProgDefLoader }
		, _texLoader{ _texDefLoader }
		, _dataMatDefLoader{ getDataLoader() }
		, _materialLoader{ _dataMatDefLoader, _progLoader, _texLoader }
		, _meshLoader{ _dataMeshDefLoader }
		, _dataArmDefLoader{ getDataLoader() }
		, _armatureLoader{ _dataArmDefLoader }
		, _dataTexAtlasDefLoader{ getDataLoader() }
		, _texPackerDefLoader{ getDataLoader(), _texDefLoader }
		, _texAtlasDefLoader{ _texPackerDefLoader, _dataTexAtlasDefLoader }
		, _texAtlasLoader{ { _texPackerDefLoader }, _texLoader }
		, _texAtlasFontLoader{ _texAtlasLoader }
		, _dataSkelAnimDefLoader{ getDataLoader() }

#ifdef DARMOK_ASSIMP
		, _assimpSceneDefLoader{ getDataLoader(), getAllocator() }
        , _sceneDefLoader{ _assimpSceneDefLoader, _dataSceneDefLoader }
#else
        , _sceneDefLoader{ _dataSceneDefLoader }
#endif
		, _dataSceneDefLoader{ getDataLoader() }
		, _sceneLoader{ _sceneDefLoader, {
			.fallback = assets
		} }
#ifdef DARMOK_OZZ
		, _ozzSkeletonLoader{ getDataLoader() }
		, _ozzSkeletalAnimationLoader{ getDataLoader() }
		, _skelLoader{ _ozzSkeletonLoader }
		, _skelAnimLoader{ _ozzSkeletalAnimationLoader }
#endif		
#ifdef DARMOK_FREETYPE
		, _freetypeFontDefLoader{ getDataLoader() }
		, _freetypeFontLoader{ _freetypeFontDefLoader, getAllocator() }
		, _fontLoader{ _freetypeFontLoader, _texAtlasFontLoader }
#else
		, _fontLoader{ _texAtlasFontLoader }
#endif
#ifdef DARMOK_MINIAUDIO
		, _miniaudioSoundLoader{ getDataLoader() }
		, _miniaudioMusicLoader{ getDataLoader() }
#endif
	{
		_fontLoader.addFront(_texAtlasFontLoader, ".xml");
	}

	bx::AllocatorI& AssetContextImpl::getAllocator() noexcept
	{
		return _config.allocator;
	}

	IDataLoader& AssetContextImpl::getDataLoader() noexcept
	{
		return _config.dataLoader;
	}
	
	IImageLoader& AssetContextImpl::getImageLoader() noexcept
	{
		return _imageLoader;
	}

	IProgramFromDefinitionLoader& AssetContextImpl::getProgramLoader() noexcept
	{
		return _progLoader;
	}

	ITextureFromDefinitionLoader& AssetContextImpl::getTextureLoader() noexcept
	{
		return _texLoader;
	}

	ITextureAtlasFromDefinitionLoader& AssetContextImpl::getTextureAtlasLoader() noexcept
	{
		return _texAtlasLoader;
	}

	IMaterialFromDefinitionLoader& AssetContextImpl::getMaterialLoader() noexcept
	{
		return _materialLoader;
	}

	IMeshFromDefinitionLoader& AssetContextImpl::getMeshLoader() noexcept
	{
		return _meshLoader;
	}

	IArmatureFromDefinitionLoader& AssetContextImpl::getArmatureLoader() noexcept
	{
		return _armatureLoader;
	}

	IFontLoader& AssetContextImpl::getFontLoader() noexcept
	{
		return _fontLoader;
	}

	ISceneLoader& AssetContextImpl::getSceneLoader() noexcept
	{
		return _sceneLoader;
	}
	
#ifdef DARMOK_OZZ
	ISkeletonLoader& AssetContextImpl::getSkeletonLoader() noexcept
	{
		return _skelLoader;
	}

	ISkeletalAnimationLoader& AssetContextImpl::getSkeletalAnimationLoader() noexcept
	{
		return _skelAnimLoader;
	}

	ISkeletalAnimatorDefinitionLoader& AssetContextImpl::getSkeletalAnimatorDefinitionLoader() noexcept
	{
		return _dataSkelAnimDefLoader;
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

	void AssetContextImpl::init(App& app)
	{
#ifdef DARMOK_FREETYPE
        _freetypeFontLoader.init(app);
#endif
	}

	void AssetContextImpl::update()
	{
		_progLoader.pruneCache();
		_texLoader.pruneCache();
		_meshLoader.pruneCache();
		_texAtlasLoader.pruneCache();
#ifdef DARMOK_FREETYPE
		_freetypeFontLoader.pruneCache();
#endif
	}

	void AssetContextImpl::shutdown()
	{
#ifdef DARMOK_FREETYPE
		_freetypeFontLoader.shutdown();
#endif
	}

	AssetContext::AssetContext(Config&& config) noexcept
		: _impl{ std::make_unique<AssetContextImpl>(*this, std::move(config)) }
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

	IProgramFromDefinitionLoader& AssetContext::getProgramLoader() noexcept
	{
		return _impl->getProgramLoader();
	}

	ITextureFromDefinitionLoader& AssetContext::getTextureLoader() noexcept
	{
		return _impl->getTextureLoader();
	}

	IMeshFromDefinitionLoader& AssetContext::getMeshLoader() noexcept
	{
		return _impl->getMeshLoader();
	}

	ITextureAtlasFromDefinitionLoader& AssetContext::getTextureAtlasLoader() noexcept
	{
		return _impl->getTextureAtlasLoader();
	}

	IMaterialFromDefinitionLoader& AssetContext::getMaterialLoader() noexcept
	{
		return _impl->getMaterialLoader();
	}

	IArmatureFromDefinitionLoader& AssetContext::getArmatureLoader() noexcept
	{
		return _impl->getArmatureLoader();
	}

	ISceneLoader& AssetContext::getSceneLoader() noexcept
	{
		return _impl->getSceneLoader();
	}

	IFontLoader& AssetContext::getFontLoader() noexcept
	{
		return _impl->getFontLoader();
	}

	ISkeletonLoader& AssetContext::getSkeletonLoader() noexcept
	{
		return _impl->getSkeletonLoader();
	}

	ISkeletalAnimationLoader& AssetContext::getSkeletalAnimationLoader() noexcept
	{
		return _impl->getSkeletalAnimationLoader();
	}

	ISkeletalAnimatorDefinitionLoader& AssetContext::getSkeletalAnimatorDefinitionLoader() noexcept
	{
		return _impl->getSkeletalAnimatorDefinitionLoader();
	}
	
	ISoundLoader& AssetContext::getSoundLoader() noexcept
	{
		return _impl->getSoundLoader();
	}

	IMusicLoader& AssetContext::getMusicLoader() noexcept
	{
		return _impl->getMusicLoader();
	}

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

	DarmokAssetFileImporter::DarmokAssetFileImporter(const CommandLineFileImporterConfig& config)
		: DarmokAssetFileImporter(config.inputPath)
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
		for (const auto& path : config.shaderIncludePaths)
		{
			addShaderIncludePath(path);
		}
	}

	DarmokAssetFileImporter::DarmokAssetFileImporter(const std::filesystem::path& inputPath)
		: _importer(inputPath)
		, _progImporter(_importer.addTypeImporter<ProgramFileImporter>())
	{
#ifdef DARMOK_ASSIMP
		_importer.addTypeImporter<AssimpSceneFileImporter>(_alloc);
#ifdef DARMOK_OZZ
		_importer.addTypeImporter<SkeletalAnimatorDefinitionFileImporter>();
		_importer.addTypeImporter<AssimpSkeletonFileImporter>();
		_importer.addTypeImporter<AssimpSkeletalAnimationFileImporter>();
#endif
#endif
#ifdef DARMOK_FREETYPE
		_importer.addTypeImporter<FreetypeFontFileImporter>();
#endif
		_importer.addTypeImporter<CopyFileImporter>();
		_importer.addTypeImporter<TexturePackerAtlasFileImporter>();
		_importer.addTypeImporter<ImageFileImporter>();
	}

	DarmokAssetFileImporter& DarmokAssetFileImporter::setCachePath(const std::filesystem::path& cachePath) noexcept
	{
		_importer.setCachePath(cachePath);
		return *this;
	}

	DarmokAssetFileImporter& DarmokAssetFileImporter::setOutputPath(const std::filesystem::path& outputPath) noexcept
	{
		_importer.setOutputPath(outputPath);
		return *this;
	}

	DarmokAssetFileImporter& DarmokAssetFileImporter::setShadercPath(const std::filesystem::path& path) noexcept
	{
		_progImporter.setShadercPath(path);
		return *this;
	}

	DarmokAssetFileImporter& DarmokAssetFileImporter::addShaderIncludePath(const std::filesystem::path& path) noexcept
	{
		_progImporter.addIncludePath(path);
		return *this;
	}

	std::vector<std::filesystem::path> DarmokAssetFileImporter::getOutputs() const
	{
		return _importer.getOutputs();
	}

	void DarmokAssetFileImporter::operator()(std::ostream& log) const
	{
		_importer(log);
	}
}
