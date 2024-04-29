#include "find.hpp"
#include <filesystem>

namespace darmok
{
    FileAssetFinder::FileAssetFinder(const std::string& basePath) noexcept
        : _basePath(basePath)
    {
    }

    std::vector<std::string> FileAssetFinder::operator()(std::string_view pattern)
    {
        std::vector<std::string> matches;
        return matches;
    }
}