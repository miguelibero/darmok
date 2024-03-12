#pragma once

#include <darmok/image.hpp>
#include <darmok/data.hpp>

namespace darmok
{
    class DataImageLoader final : public IImageLoader
	{
	public:
		DataImageLoader(IDataLoader& dataLoader, bx::AllocatorI* alloc) noexcept;
		[[nodiscard]] std::shared_ptr<Image> operator()(std::string_view name) override;
	private:
		IDataLoader& _dataLoader;
		bx::AllocatorI* _allocator;
	};
}