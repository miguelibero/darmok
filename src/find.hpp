#pragma once

#include <darmok/find.hpp>

namespace darmok
{
    class FileAssetFinder final : public IAssetFinder
    {
    public:
        FileAssetFinder(std::string_view basePath = "") noexcept;
        void setBasePath(std::string_view basePath) noexcept;
        std::vector<std::string> operator()(std::string_view pattern) override;
    private:
        std::string _basePath;
    };
}