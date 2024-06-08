#pragma once

#include <darmok/vertex_layout.hpp>

namespace darmok
{
    class IDataLoader;

    class BinaryVertexLayoutLoader final : public IVertexLayoutLoader
    {
    public:
        BinaryVertexLayoutLoader(IDataLoader& dataLoader) noexcept;
        bgfx::VertexLayout operator()(std::string_view name) override;
    private:
        IDataLoader& _dataLoader;
    };
}