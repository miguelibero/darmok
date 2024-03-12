#pragma once

#include <darmok/model.hpp>
#include <darmok/optional_ref.hpp>
#include <assimp/Importer.hpp>

namespace darmok
{
    class AssimpModelLoader final : public IModelLoader
	{
	public:
		AssimpModelLoader(IDataLoader& dataLoader, const OptionalRef<ITextureLoader>& textureLoader = nullptr, bx::AllocatorI* alloc = nullptr);
		std::shared_ptr<Model> operator()(std::string_view name) override;
	private:
		Assimp::Importer _importer;
		IDataLoader& _dataLoader;
		OptionalRef<ITextureLoader> _textureLoader;
		bx::AllocatorI* _alloc;
	};
}