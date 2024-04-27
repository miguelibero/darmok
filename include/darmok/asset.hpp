#pragma once

#include <memory>

namespace darmok
{
	class AssetContextImpl;
	class IDataLoader;
	class IImageLoader;
	class IProgramLoader;
	class StandardProgramLoader;
	class ITextureLoader;
	class ITextureAtlasLoader;
	class IVertexLayoutLoader;
	class ColorTextureLoader;
    class AssetContext;

#ifdef DARMOK_ASSIMP 
	class AssimpSceneLoader;
#endif

	class AssetContext final
	{
	public:
		AssetContext() noexcept;
		~AssetContext() noexcept;

		[[nodiscard]] IDataLoader& getDataLoader() noexcept;
		[[nodiscard]] IImageLoader& getImageLoader() noexcept;
		[[nodiscard]] IProgramLoader& getProgramLoader() noexcept;
		[[nodiscard]] StandardProgramLoader& getStandardProgramLoader() noexcept;
		[[nodiscard]] ITextureLoader& getTextureLoader() noexcept;
		[[nodiscard]] ITextureAtlasLoader& getTextureAtlasLoader() noexcept;
		[[nodiscard]] ColorTextureLoader& getColorTextureLoader() noexcept;

#ifdef DARMOK_ASSIMP 
		[[nodiscard]] AssimpSceneLoader& getAssimpLoader() noexcept;
#endif

		[[nodiscard]] AssetContextImpl& getImpl() noexcept;
		[[nodiscard]] const AssetContextImpl& getImpl() const noexcept;
	private:
		std::unique_ptr<AssetContextImpl> _impl;
	};
}