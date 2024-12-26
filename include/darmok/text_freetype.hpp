#pragma once

#include <darmok/export.h>
#include <darmok/text.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/glm_serialize.hpp>
#include <darmok/data.hpp>

#include <cereal/cereal.hpp>

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
    class IDataLoader;
    class FreetypeFontLoaderImpl;
    class App;

    struct FreetypeFontDefinition final
    {
        Data data;
        glm::uvec2 fontSize = glm::uvec2(48, 48);

        template<typename Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(data),
                CEREAL_NVP(fontSize)
            );
        }
    };

    class DARMOK_EXPORT BX_NO_VTABLE IFreetypeFontDefinitionLoader : public ILoader<FreetypeFontDefinition>
    {
    };

    class DARMOK_EXPORT DataFreetypeFontDefinitionLoader : public IFreetypeFontDefinitionLoader
    {
    public:
        DataFreetypeFontDefinitionLoader(IDataLoader& dataLoader, bool skipInvalid = true) noexcept;
        [[nodiscard]] std::shared_ptr<FreetypeFontDefinition> operator()(std::filesystem::path path) override;
        static std::string checkFontData(const DataView& data) noexcept;
    private:
        IDataLoader& _dataLoader;
        bool _skipInvalid;
    };

    using CerealFreetypeFontDefinitionLoader = CerealLoader<IFreetypeFontDefinitionLoader>;

    class DARMOK_EXPORT FreetypeFontLoader final : public FromDefinitionLoader<IFontLoader, IFreetypeFontDefinitionLoader>
    {
    public:
        FreetypeFontLoader(IFreetypeFontDefinitionLoader& defLoader, bx::AllocatorI& alloc);
        ~FreetypeFontLoader() noexcept;
        void init(App& app);
        void shutdown();
    protected:
        std::shared_ptr<IFont> create(const std::shared_ptr<FreetypeFontDefinition>& def) override;
    private:
        std::unique_ptr<FreetypeFontLoaderImpl> _impl;
    };

    class FreetypeFontAtlasFileImporterImpl;

    class DARMOK_EXPORT FreetypeFontAtlasFileImporter final : public IFileTypeImporter
    {
    public:
        FreetypeFontAtlasFileImporter() noexcept;
        ~FreetypeFontAtlasFileImporter() noexcept;
        std::vector<std::filesystem::path> getOutputs(const Input& input) override;
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path) override;
        bool startImport(const Input& input, bool dry = false) override;
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
        const std::string& getName() const noexcept override;
        void endImport(const Input& input) override;
    private:
        std::unique_ptr<FreetypeFontAtlasFileImporterImpl> _impl;
    };
}