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
        [[nodiscard]] Result operator()(std::filesystem::path path) noexcept override;
        static expected<void, std::string> checkFontData(const DataView& data) noexcept;
    private:
        IDataLoader& _dataLoader;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IFreetypeFontLoader : public IFromDefinitionLoader<IFontLoader, protobuf::FreetypeFont>
    {
    };

    class DARMOK_EXPORT FreetypeFontLoader final : public FromDefinitionLoader<IFreetypeFontLoader, IFreetypeFontDefinitionLoader>
    {
    public:
        FreetypeFontLoader(IFreetypeFontDefinitionLoader& defLoader, bx::AllocatorI& alloc) noexcept;
        ~FreetypeFontLoader() noexcept;
        expected<void, std::string> init(App& app) noexcept;
        expected<void, std::string> shutdown() noexcept;

        static protobuf::FreetypeFont createDefinition() noexcept;
    protected:
        Result create(std::shared_ptr<Definition> def) noexcept override;
    private:
        std::unique_ptr<FreetypeFontLoaderImpl> _impl;
    };

    class FreetypeFontFileImporterImpl;

    class DARMOK_EXPORT FreetypeFontFileImporter final : public IFileTypeImporter
    {
    public:
        FreetypeFontFileImporter() noexcept;
        ~FreetypeFontFileImporter() noexcept;
        const std::string& getName() const noexcept override;

		expected<void, std::string> init(OptionalRef<std::ostream> log) noexcept override;
        expected<void, std::string> shutdown() noexcept override;
        expected<Effect, std::string> prepare(const Input& input) noexcept override;
        expected<void, std::string> operator()(const Input& input, Config& config) noexcept override;
    private:
        std::unique_ptr<FreetypeFontFileImporterImpl> _impl;
    };
}