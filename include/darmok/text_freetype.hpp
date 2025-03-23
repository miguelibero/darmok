#pragma once

#include <darmok/export.h>
#include <darmok/text.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/glm_serialize.hpp>
#include <darmok/data.hpp>
#include <darmok/protobuf/text.pb.h>

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
    class IDataLoader;
    class FreetypeFontLoaderImpl;
    class App;

    class DARMOK_EXPORT BX_NO_VTABLE IFreetypeFontDefinitionLoader : public ILoader<protobuf::FreetypeFont>
    {
    };

    class DARMOK_EXPORT FreetypeFontDefinitionLoader : public IFreetypeFontDefinitionLoader
    {
    public:
        FreetypeFontDefinitionLoader(IDataLoader& dataLoader) noexcept;
        [[nodiscard]] Result operator()(std::filesystem::path path) override;
        static std::string checkFontData(const DataView& data) noexcept;
    private:
        IDataLoader& _dataLoader;
    };

    class DARMOK_EXPORT FreetypeFontLoader final : public FromDefinitionLoader<IFontLoader, IFreetypeFontDefinitionLoader>
    {
    public:
        FreetypeFontLoader(IFreetypeFontDefinitionLoader& defLoader, bx::AllocatorI& alloc);
        ~FreetypeFontLoader() noexcept;
        void init(App& app);
        void shutdown();
    protected:
        Result create(const std::shared_ptr<Definition>& def) override;
    private:
        std::unique_ptr<FreetypeFontLoaderImpl> _impl;
    };

    class FreetypeFontFileImporterImpl;

    class DARMOK_EXPORT FreetypeFontFileImporter final : public IFileTypeImporter
    {
    public:
        FreetypeFontFileImporter() noexcept;
        ~FreetypeFontFileImporter() noexcept;
        std::vector<std::filesystem::path> getOutputs(const Input& input) override;
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path) override;
        bool startImport(const Input& input, bool dry = false) override;
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
        const std::string& getName() const noexcept override;
        void endImport(const Input& input) override;
    private:
        std::unique_ptr<FreetypeFontFileImporterImpl> _impl;
    };
}