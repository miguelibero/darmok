#pragma once

#include <darmok/export.h>
#include <darmok/text.hpp>
#include <darmok/asset_core.hpp>

namespace darmok
{
    class IDataLoader;
    class FreetypeFontLoaderImpl;

    class DARMOK_EXPORT FreetypeFontLoader final : public IFontLoader
    {
    public:
        FreetypeFontLoader(IDataLoader& dataLoader);
        ~FreetypeFontLoader() noexcept;
        std::shared_ptr<Font> operator()(std::string_view name) override;
    private:
        std::unique_ptr<FreetypeFontLoaderImpl> _impl;
    };

    class FreetypeFontAtlasImporterImpl;

    class DARMOK_EXPORT FreetypeFontAtlasImporter final : public IAssetTypeImporter
    {
    public:
        FreetypeFontAtlasImporter() noexcept;
        ~FreetypeFontAtlasImporter() noexcept;
        std::vector<std::filesystem::path> getOutputs(const Input& input) override;
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path) override;
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
        const std::string& getName() const noexcept override;
    private:
        std::unique_ptr<FreetypeFontAtlasImporterImpl> _impl;
    };
}