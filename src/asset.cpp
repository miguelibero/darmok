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

	void FileReader::setBasePath(const std::filesystem::path& basePath) noexcept
	{
		_basePath = basePath;
	}

	bool FileReader::open(const bx::FilePath& filePath, bx::Error* err)
	{
		auto path = fixAssetFilePath(filePath, _basePath).string();
		return bx::FileReader::open(path.c_str(), err);
	}

	void FileWriter::setBasePath(const std::filesystem::path& basePath) noexcept
	{
		_basePath = basePath;
	}

	bool FileWriter::open(const bx::FilePath& filePath, bool append, bx::Error* err)
	{
		auto path = fixAssetFilePath(filePath, _basePath).string();
		return bx::FileWriter::open(path.c_str(), append, err);
	}

	AssetContextImpl::AssetContextImpl()
		: _dataLoader(_fileReader, _allocator)
		, _imageLoader(_dataLoader, _allocator)
		, _vertexLayoutLoader(_dataLoader)
		, _programLoader(_dataLoader, _vertexLayoutLoader)
		, _textureLoader(_imageLoader)
		, _textureAtlasLoader(_dataLoader, _textureLoader)
		, _colorTextureLoader(_allocator)
		, _binModelLoader(_dataLoader)
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
	{
		setBasePath("assets");

#ifdef DARMOK_OZZ
		_skeletonLoader.setDefaultLoader(_ozzSkeletonLoader);
		_skeletalAnimationLoader.setDefaultLoader(_ozzSkeletalAnimationLoader);
#endif

		_modelLoader.setDefaultLoader(_binModelLoader);
		_modelLoader.addLoader(".dml;.bin", _binModelLoader);

#ifdef DARMOK_ASSIMP
		_modelLoader.setDefaultLoader(_assimpModelLoader);
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

	StandardProgramLoader& AssetContextImpl::getStandardProgramLoader() noexcept
	{
		return _standardProgramLoader;
	}

	ITextureLoader& AssetContextImpl::getTextureLoader() noexcept
	{
		return _textureLoader;
	}

	ColorTextureLoader& AssetContextImpl::getColorTextureLoader() noexcept
	{
		return _colorTextureLoader;
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
		return _freetypeFontLoader;
	}
#endif

	void AssetContextImpl::setBasePath(const std::filesystem::path& path) noexcept
	{
		_fileReader.setBasePath(path);
		_fileWriter.setBasePath(path);
	}

	void AssetContextImpl::init(App& app)
	{
		_freetypeFontLoader.init(app);
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

	StandardProgramLoader& AssetContext::getStandardProgramLoader() noexcept
	{
		return _impl->getStandardProgramLoader();
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

	ColorTextureLoader& AssetContext::getColorTextureLoader() noexcept
	{
		return _impl->getColorTextureLoader();
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

	AssetContext& AssetContext::setBasePath(const std::filesystem::path& path) noexcept
	{
		_impl->setBasePath(path);
		return *this;
	}

	void AssetContext::init(App& app)
	{
		_impl->init(app);
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
		, _shaderImporter(_importer.addTypeImporter<ShaderImporter>())
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
		_importer.addTypeImporter<VertexLayoutImporter>();
		_importer.addTypeImporter<CopyAssetImporter>();
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
		_shaderImporter.setShadercPath(path);
		return *this;
	}

	DarmokAssetImporter& DarmokAssetImporter::addShaderIncludePath(const std::filesystem::path& path) noexcept
	{
		_shaderImporter.addIncludePath(path);
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
