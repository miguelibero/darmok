#pragma once

#include <darmok/vertex.hpp>
#include <darmok/data.hpp>

namespace darmok
{
    class JsonDataVertexLayoutLoader final : public IVertexLayoutLoader
    {
    public:
        JsonDataVertexLayoutLoader(IDataLoader& dataLoader) noexcept;
        bgfx::VertexLayout operator()(std::string_view name) override;
    private:
        IDataLoader& _dataLoader;
    };
}