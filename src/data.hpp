#pragma once

#include <darmok/data.hpp>
#include "find.hpp"
#include <bx/file.h>

namespace darmok
{
    class FileDataLoader final : public IDataLoader
	{
	public:
		FileDataLoader(bx::FileReaderI* fileReader, bx::AllocatorI* alloc = nullptr);
		IDataLoader::result_type operator()(std::string_view filePath) override;
		std::vector<std::string> find(std::string_view name) override;
	private:
		bx::FileReaderI* _fileReader;
		bx::AllocatorI* _allocator;
		FileAssetFinder _finder;
	};
}