#include "find.hpp"
#include <filesystem>

namespace darmok
{
    FileAssetFinder::FileAssetFinder(std::string_view basePath) noexcept
        : _basePath(basePath)
    {
    }

    void FileAssetFinder::setBasePath(std::string_view basePath) noexcept
    {
        _basePath = basePath;
    }

    std::vector<std::string> FileAssetFinder::operator()(std::string_view pattern)
    {
        std::vector<std::string> matches;
        return matches;
    }
}