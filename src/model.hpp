#pragma once

#include <darmok/model.hpp>
#include <assimp/Importer.hpp>

namespace darmok
{
    class AssimpModelLoader final : public IModelLoader
	{
	public:
		AssimpModelLoader(IDataLoader& dataLoader, ITextureLoader& textureLoader);
		std::shared_ptr<Model> operator()(std::string_view name) override;
	private:
		Assimp::Importer _importer;
		IDataLoader& _dataLoader;
		ITextureLoader& _textureLoader;
	};
}