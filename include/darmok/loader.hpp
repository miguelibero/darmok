#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <darmok/string.hpp>
#include <darmok/serialize.hpp>
#include <darmok/data.hpp>

#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <fstream>
#include <memory>

namespace darmok
{
    template<typename Type>
    class DARMOK_EXPORT ILoader
    {
    public:
        using Resource = Type;
        virtual ~ILoader() = default;
        [[nodiscard]] virtual std::shared_ptr<Resource> operator()(const std::filesystem::path& path) = 0;
    };

    template<typename Interface>
    class DARMOK_EXPORT ExtensionLoader final : public Interface
    {
    public:
        using Resource = typename Interface::Resource;

        ExtensionLoader(Interface& defaultLoader)
            : _defaultLoader(defaultLoader)
        {
        }

        void addLoader(const std::string& exts, Interface& loader) noexcept
        {
            _loaders.emplace_back(StringUtils::split(exts, ";"), loader);
        }

        std::shared_ptr<Resource> operator()(const std::filesystem::path& path) override
        {
            auto ext = path.extension();
            for (auto& elm : _loaders)
            {
                auto itr = std::find(elm.exts.begin(), elm.exts.end(), ext);
                if (itr != elm.exts.end())
                {
                    return elm.loader.get()(path);
                }
            }
            return _defaultLoader(path);
        }
    private:
        struct Element final
        {
            std::vector<std::string> exts;
            std::reference_wrapper<Interface> loader;
        };

        std::vector<Element> _loaders;
        Interface& _defaultLoader;
    };

    template<typename Type, typename DefinitionType>
    class DARMOK_EXPORT IFromDefinitionLoader : public ILoader<Type>
    {
    public:
        using Resource = Type;
        using Definition = DefinitionType;
        virtual std::shared_ptr<Resource> forceLoad(const std::filesystem::path& path) = 0;
        virtual std::shared_ptr<Definition> getDefinition(const std::shared_ptr<Resource>& res) = 0;
        virtual std::shared_ptr<Resource> loadResource(const std::shared_ptr<Definition>& def, bool force = false) = 0;
        virtual std::shared_ptr<Definition> loadDefinition(const std::filesystem::path& path, bool force = false) = 0;

        virtual bool clearCache(const std::filesystem::path& path) = 0;
        virtual void clearCache() = 0;
        virtual void pruneCache() = 0;
    };

    template<typename Interface, typename DefinitionLoader>
    class DARMOK_EXPORT FromDefinitionLoader : public Interface
    {
    public:
        using Resource = Interface::Resource;
        using Definition = DefinitionLoader::Resource;

        FromDefinitionLoader(DefinitionLoader& defLoader)
            : _defLoader(defLoader)
        {
        }

        std::shared_ptr<Resource> operator()(const std::filesystem::path& path)
        {
            auto def = loadDefinition(path);
            return loadResource(def);
        }

        std::shared_ptr<Resource> forceLoad(const std::filesystem::path& path)
        {
            auto def = loadDefinition(path, true);
            return loadResource(def, true);
        }

        std::shared_ptr<Definition> loadDefinition(const std::filesystem::path& path, bool force = false)
        {
            if (!force)
            {
                auto itr = _defCache.find(path);
                if (itr != _defCache.end())
                {
                    return itr->second;
                }
            }
            auto def = _defLoader(path);
            _defCache[path] = def;
            return def;
        }

        std::shared_ptr<Resource> loadResource(const std::shared_ptr<Definition>& def, bool force = false)
        {
            if (!force)
            {
                auto itr = _resCache.find(def);
                if (itr != _resCache.end())
                {
                    if (auto ptr = itr->second.lock())
                    {
                        return ptr;
                    }
                }
            }

            std::shared_ptr<Resource> res;
            if constexpr (std::is_abstract_v<Resource>)
            {
                res = Resource::create(*def);
            }
            else
            {
                res = std::make_shared<Resource>(*def);
            }
            _resCache[def] = res;
            return res;
        }

        std::shared_ptr<Definition> getDefinition(const std::shared_ptr<Resource>& res) noexcept
        {
            auto itr = std::find_if(_resCache.begin(), _resCache.end(), [res](auto& elm) {
                return elm.second.lock() == res;
            });
            if (itr != _resCache.end())
            {
                return itr->first;
            }
            return nullptr;
        }

        bool clearCache(const std::filesystem::path& path) noexcept
        {
            auto itr = _defCache.find(path);
            if (itr == _defCache.end())
            {
                return false;
            }
            _defCache.erase(itr);
            auto itr2 = _resCache.find(itr->second);
            if (itr2 != _resCache.end())
            {
                _resCache.erase(itr2);
            }
            return true;
        }

        void clearCache() noexcept
        {
            _defCache.clear();
            _resCache.clear();
        }

        void pruneCache() noexcept
        {
            // remove cache items that don't have any references
            for (auto itr = _resCache.begin(); itr != _resCache.end();)
            {
                if (itr->second.lock() == nullptr)
                {
                    itr = _resCache.erase(itr);
                }
                else {
                    ++itr;
                }
            }
            for (auto itr = _defCache.begin(); itr != _defCache.end();)
            {
                if (!_resCache.contains(itr->second))
                {
                    itr = _defCache.erase(itr);
                }
                else {
                    ++itr;
                }
            }
        }

    private:
        DefinitionLoader& _defLoader;
        std::unordered_map<std::filesystem::path, std::shared_ptr<Definition>> _defCache;
        std::unordered_map<std::shared_ptr<Definition>, std::weak_ptr<Resource>> _resCache;
    };

    template<typename Interface>
    class DARMOK_EXPORT CerealLoader : public Interface
    {
    public:
        using Resource = Interface::Resource;

        CerealLoader(IDataLoader& dataLoader) noexcept
            : _dataLoader(dataLoader)
        {
        }

        virtual std::shared_ptr<Resource> operator()(const std::filesystem::path& path)
        {
            auto data = _dataLoader(path);
            auto format = CerealUtils::getExtensionFormat(path.extension());
            auto res = std::make_shared<Resource>();
            CerealUtils::load(*res, data, format);
            return res;
        }
    private:
        IDataLoader& _dataLoader;
    };
}