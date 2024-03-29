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
	class IProgramDefinitionLoader;
	class ColorTextureLoader;

	class AssetContext final
	{
	public:
		AssetContext() noexcept;
		~AssetContext() noexcept;

		[[nodiscard]] IImageLoader& getImageLoader() noexcept;
		[[nodiscard]] IProgramLoader& getProgramLoader() noexcept;
		[[nodiscard]] IProgramDefinitionLoader& getProgramDefinitionLoader() noexcept;
		[[nodiscard]] ITextureLoader& getTextureLoader() noexcept;
		[[nodiscard]] ITextureAtlasLoader& getTextureAtlasLoader() noexcept;
		[[nodiscard]] IModelLoader& getModelLoader() noexcept;
		[[nodiscard]] IVertexLayoutLoader& getVertexLayoutLoader() noexcept;
		[[nodiscard]] ColorTextureLoader& getColorTextureLoader() noexcept;

		[[nodiscard]] AssetContextImpl& getImpl() noexcept;
		[[nodiscard]] const AssetContextImpl& getImpl() const noexcept;
	private:
		std::unique_ptr<AssetContextImpl> _impl;
	};
}