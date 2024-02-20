#pragma once

#include <darmok/data.hpp>
#include <bx/file.h>

namespace darmok
{
    class FileDataLoader final : public IDataLoader
	{
	public:
		FileDataLoader(bx::FileReaderI* fileReader, bx::AllocatorI* alloc = nullptr);
		std::shared_ptr<Data> operator()(std::string_view filePath) override;
	private:
		bx::FileReaderI* _fileReader;
		bx::AllocatorI* _allocator;
	};
}