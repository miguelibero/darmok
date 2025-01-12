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
		, _cerealProgDefLoader(_dataLoader)
		, _cerealTexDefLoader(_dataLoader)
		, _imgTexDefLoader(_imageLoader)
		, _texDefLoader{ _imgTexDefLoader, _cerealTexDefLoader }
		, _cerealMeshDefLoader(_dataLoader)
		, _progLoader(_cerealProgDefLoader)
		, _texLoader(_texDefLoader)
		, _cerealMaterialLoader(_dataLoader)
		, _meshLoader(_cerealMeshDefLoader)
		, _cerealTexAtlasDefLoader(_dataLoader)
		, _texPackerDefLoader(_dataLoader, _texDefLoader)
		, _texAtlasDefLoader{ _texPackerDefLoader, _cerealTexAtlasDefLoader }
		, _texAtlasLoader({ _texPackerDefLoader }, _texLoader)
		, _cerealModelLoader(_dataLoader)
		, _texAtlasFontLoader(_texAtlasLoader)
#ifdef DARMOK_OZZ
		, _skelAnimDefLoader(_dataLoader)
		, _ozzSkeletonLoader(_dataLoader)
		, _ozzSkeletalAnimationLoader(_dataLoader)
		, _skelLoader(_ozzSkeletonLoader)
		, _skelAnimLoader(_ozzSkeletalAnimationLoader)
#endif		
#ifdef DARMOK_ASSIMP
		, _assimpModelLoader(_dataLoader, _allocator, _imageLoader)
		, _modelLoader(_assimpModelLoader)
#else
		, _modelLoader(_cerealModelLoader)
#endif		
#ifdef DARMOK_FREETYPE
		, _dataFreetypeFontDefLoader(_dataLoader)
		, _cerealFreetypeFontDefLoader(_dataLoader)
		, _freetypeFontDefLoader({ _dataFreetypeFontDefLoader, _cerealFreetypeFontDefLoader })
		, _freetypeFontLoader(_freetypeFontDefLoader, _allocator)
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
		_modelLoader.addFront(_cerealModelLoader, ".dml;.bin");
		_fontLoader.addFront(_texAtlasFontLoader, ".xml");
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
		return _progLoader;
	}

	ITextureLoader& AssetContextImpl::getTextureLoader() noexcept
	{
		return _texLoader;
	}

	ITextureAtlasLoader& AssetContextImpl::getTextureAtlasLoader() noexcept
	{
		return _texAtlasLoader;
	}

	IMaterialLoader& AssetContextImpl::getMaterialLoader() noexcept
	{
		return _cerealMaterialLoader;
	}

	IMeshLoader& AssetContextImpl::getMeshLoader() noexcept
	{
		return _meshLoader;
	}

	IModelLoader& AssetContextImpl::getModelLoader() noexcept
	{
		return _modelLoader;
	}

	IFontLoader& AssetContextImpl::getFontLoader() noexcept
	{
		return _fontLoader;
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
		return _skelLoader;
	}

	ISkeletalAnimationLoader& AssetContextImpl::getSkeletalAnimationLoader() noexcept
	{
		return _skelAnimLoader;
	}

	ISkeletalAnimatorDefinitionLoader& AssetContextImpl::getSkeletalAnimatorDefinitionLoader() noexcept
	{
		return _skelAnimDefLoader;
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

	IMaterialLoader& AssetContext::getMaterialLoader() noexcept
	{
		return _impl->getMaterialLoader();
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

	ISkeletalAnimatorDefinitionLoader& AssetContext::getSkeletalAnimatorDefinitionLoader() noexcept
	{
		return _impl->getSkeletalAnimatorDefinitionLoader();
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
		for (auto& path : config.shaderIncludePaths)
		{
			addShaderIncludePath(path);
		}
	}

	DarmokAssetFileImporter::DarmokAssetFileImporter(const std::filesystem::path& inputPath)
		: _importer(inputPath)
		, _progImporter(_importer.addTypeImporter<ProgramFileImporter>())
	{
#ifdef DARMOK_ASSIMP
		_importer.addTypeImporter<AssimpModelFileImporter>(_alloc);
#ifdef DARMOK_OZZ
		_importer.addTypeImporter<SkeletalAnimatorDefinitionFileImporter>();
		_importer.addTypeImporter<AssimpSkeletonFileImporter>();
		_importer.addTypeImporter<AssimpSkeletalAnimationFileImporter>();
#endif
#endif
#ifdef DARMOK_FREETYPE
		_importer.addTypeImporter<FreetypeFontAtlasFileImporter>();
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
		return _importer(log);
	}
}
