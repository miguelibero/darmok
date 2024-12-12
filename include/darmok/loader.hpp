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

#include <bx/bx.h>

namespace darmok
{
    template<typename Type>
    class DARMOK_EXPORT BX_NO_VTABLE ILoader
    {
    public:
        using Resource = Type;
        virtual ~ILoader() = default;
        [[nodiscard]] virtual std::shared_ptr<Resource> operator()(const std::filesystem::path& path) = 0;
    };

    template<typename Interface>
    class DARMOK_EXPORT ContainerLoader final : public Interface
    {
    public:
        using Resource = typename Interface::Resource;

        struct Element final
        {
            std::reference_wrapper<Interface> loader;
            std::vector<std::string> exts;
        };
        using Elements = std::vector<Element>;

        ContainerLoader(Interface& loader) noexcept
        {
            _loaders.emplace_back(loader);
        }

        ContainerLoader(std::initializer_list<std::reference_wrapper<Interface>> loaders) noexcept
        {
            _loaders.reserve(loaders.size());
            for (auto& loader : loaders)
            {
                _loaders.emplace_back(loader);
            }
        }

        ContainerLoader(const Elements& elements) noexcept
            : _loaders(elements)
        {
        }

        ContainerLoader& addFront(Interface& loader, const std::string& exts = "") noexcept
        {
            _loaders.emplace(_loaders.begin(), loader, StringUtils::split(exts, ";"));
            return *this;
        }

        ContainerLoader& addBack(Interface& loader, const std::string& exts = "") noexcept
        {
            _loaders.emplace_back(loader, StringUtils::split(exts, ";"));
            return *this;
        }

        std::shared_ptr<Resource> operator()(const std::filesystem::path& path) override
        {
            auto ext = path.extension();
            for (auto& [loader, exts] : _loaders)
            {
                if (!exts.empty())
                {
                    auto itr = std::find(exts.begin(), exts.end(), ext);
                    if (itr == exts.end())
                    {
                        continue;
                    }
                }
                if (auto res = loader.get()(path))
                {
                    return res;
                }
            }
            return nullptr;
        }

    private:
        Elements _loaders;
    };

    template<typename Type>
    class DARMOK_EXPORT BX_NO_VTABLE ICachedLoader : public ILoader<Type>
    {
        virtual std::shared_ptr<Type> forceLoad(const std::filesystem::path& path) = 0;

        virtual bool clearCache(const std::filesystem::path& path) = 0;
        virtual void clearCache() = 0;
        virtual void pruneCache() = 0;
    };

    template<typename Type>
    class DARMOK_EXPORT CachedLoader final : public ICachedLoader<Type>
    {
    public:
        using Resource = Type;

        std::shared_ptr<Resource> forceLoad(const std::filesystem::path& path) override
        {
            auto res = doLoad(path);
            _cache[path] = res;
            return res;
        }

        std::shared_ptr<Resource> operator()(const std::filesystem::path& path)
        {
            auto itr = _cache.find(path);
            if (itr != _cache.end())
            {
                if (auto res = itr->second.lock())
                {
                    return res;
                }
            }
            return forceLoad(path);
        }

        bool clearCache(const std::filesystem::path& path) override
        {
            auto itr = _cache.find(path);
            if (itr == _cache.end())
            {
                return false;
            }
            _cache.erase(itr);
            return true;
        }

        void clearCache() override
        {
            _cache.clear();
        }

        void pruneCache() override
        {
            // remove cache items that don't have any references
            for (auto itr = _cache.begin(); itr != _cache.end();)
            {
                if (itr->second.lock() == nullptr)
                {
                    itr = _cache.erase(itr);
                }
                else {
                    ++itr;
                }
            }
        }
    protected:
        virtual std::shared_ptr<Resource> doLoad(const std::filesystem::path& path) = 0;
    private:
        std::unordered_map<std::filesystem::path, std::weak_ptr<Resource>> _cache;
    };

    template<typename Type, typename DefinitionType>
    class DARMOK_EXPORT BX_NO_VTABLE IFromDefinitionLoader : public ICachedLoader<Type>
    {
    public:
        using Resource = Type;
        using Definition = DefinitionType;
        virtual std::shared_ptr<Definition> getDefinition(const std::shared_ptr<Resource>& res) = 0;
        virtual std::shared_ptr<Resource> loadResource(const std::shared_ptr<Definition>& def, bool force = false) = 0;
        virtual std::shared_ptr<Definition> loadDefinition(const std::filesystem::path& path, bool force = false) = 0;
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
            if (auto def = _defLoader(path))
            {
                _defCache[path] = def;
                return def;
            }
            return nullptr;
        }

        std::shared_ptr<Resource> loadResource(const std::shared_ptr<Definition>& def, bool force = false)
        {
            if (!force)
            {
                auto itr = _resCache.find(def);
                if (itr != _resCache.end())
                {
                    if (auto res = itr->second.lock())
                    {
                        return res;
                    }
                }
            }
            auto res = create(def);
            _resCache[def] = res;
            return res;
        }

        std::shared_ptr<Definition> getDefinition(const std::shared_ptr<Resource>& res) noexcept
        {
            auto itr = std::find_if(_resCache.begin(), _resCache.end(),
                [res](auto& elm) {
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

    protected:
        virtual std::shared_ptr<Resource> create(const std::shared_ptr<Definition>& def)
        {
            if constexpr (std::is_constructible_v<Resource, Definition>)
            {
                return std::make_shared<Resource>(*def);
            }
            else if constexpr (std::is_constructible_v<Resource, std::shared_ptr<Definition>>)
            {
                return std::make_shared<Resource>(def);
            }
            return nullptr;
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