#pragma once

#include <darmok/model.hpp>

namespace darmok
{
    class BinaryModelLoader : public IModelLoader
	{
	public:
        BinaryModelLoader(IDataLoader& dataLoader) noexcept;
        DLLEXPORT std::shared_ptr<Model> operator()(std::string_view name) override;
    private:
        IDataLoader& _dataLoader;
	};
}