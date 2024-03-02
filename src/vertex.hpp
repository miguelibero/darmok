#pragma once

#include <darmok/vertex.hpp>
#include <darmok/data.hpp>

namespace darmok
{
    class XmlDataVertexLayoutLoader final : public IVertexLayoutLoader
	{
	public:
		XmlDataVertexLayoutLoader(IDataLoader& dataLoader);
		bgfx::VertexLayout operator()(std::string_view name) override;
	private:
		IDataLoader& _dataLoader;
	};
}