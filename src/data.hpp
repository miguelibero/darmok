#pragma once

#include <darmok/data.hpp>
#include <bx/file.h>

namespace darmok
{
    class FileDataLoader final : public IDataLoader
	{
	public:
		FileDataLoader(bx::FileReaderI* fileReader, const OptionalRef<bx::AllocatorI>& alloc = nullptr);
		IDataLoader::result_type operator()(std::string_view filePath) override;
	private:
		bx::FileReaderI* _fileReader;
		OptionalRef<bx::AllocatorI> _allocator;
	};
}