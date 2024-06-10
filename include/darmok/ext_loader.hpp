#pragma once

#include <vector>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <darmok/optional_ref.hpp>
#include <darmok/string.hpp>

namespace darmok
{
    template<typename L>
    class ExtensionLoader final : public L
    {
    public:
        void setDefaultLoader(L& defaultLoader) noexcept
        {
            _defaultLoader = defaultLoader;
        }

        void addLoader(const std::string& exts, L& loader) noexcept
        {
            _loaders.push_back({
                StringUtils::split(exts, ","),
                loader
            });
        }

        L::result_type operator()(std::string_view path) override
        {
            auto ext = std::filesystem::path(path).extension();
            for (auto& elm : _loaders)
            {
                auto itr = std::find(elm.exts.begin(), elm.exts.end(), ext);
                if (itr != elm.exts.end())
                {
                    return (*elm.loader)(path);
                }
            }
            if (_defaultLoader)
            {
                return (*_defaultLoader)(path);
            }
            throw std::runtime_error("could not find a valid loader");
        }
    private:
        struct Element final
        {
            std::vector<std::string> exts;
            OptionalRef<L> loader;
        };

        std::vector<Element> _loaders;
        OptionalRef<L> _defaultLoader;
    };
}