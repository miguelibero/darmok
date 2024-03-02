#pragma once

#include <memory>

namespace darmok
{
	class AssetContextImpl;
	class IImageLoader;
	class IProgramLoader;
	class ITextureLoader;
	class ITextureAtlasLoader;
	class IModelLoader;
	class IVertexLayoutLoader;

	class AssetContext final
	{
	public:
		static AssetContext& get() noexcept;

		IImageLoader& getImageLoader() noexcept;
		IProgramLoader& getProgramLoader() noexcept;
		ITextureLoader& getTextureLoader() noexcept;
		ITextureAtlasLoader& getTextureAtlasLoader() noexcept;
		IModelLoader& getModelLoader() noexcept;
		IVertexLayoutLoader& getVertexLayoutLoader() noexcept;

		AssetContextImpl& getImpl() noexcept;
		const AssetContextImpl& getImpl() const noexcept;
	private:
		AssetContext() noexcept;
		std::unique_ptr<AssetContextImpl> _impl;
	};
}