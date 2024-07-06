#pragma once

#include <darmok/export.h>
#include <darmok/text.hpp>

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
}