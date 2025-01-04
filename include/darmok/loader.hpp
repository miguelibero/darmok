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
    template<typename Type, typename Arg>
    class DARMOK_EXPORT BX_NO_VTABLE IBasicLoader
    {
    public:
        using Resource = Type;
        virtual ~IBasicLoader() = default;
        [[nodiscard]] virtual std::shared_ptr<Resource> operator()(Arg arg) = 0;
    };

    template<typename Type>
    using ILoader = IBasicLoader<Type, std::filesystem::path>;

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

        void addFront(Interface& loader, const std::string& exts = "") noexcept
        {
            _loaders.emplace(_loaders.begin(), loader, StringUtils::split(exts, ";"));
        }

        void addBack(Interface& loader, const std::string& exts = "") noexcept
        {
            _loaders.emplace_back(loader, StringUtils::split(exts, ";"));
        }

        std::shared_ptr<Resource> operator()(std::filesystem::path path) override
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

    template<typename Type, typename Arg>
    class DARMOK_EXPORT BX_NO_VTABLE IBasicCachedLoader : public IBasicLoader<Type, Arg>
    {
        virtual std::shared_ptr<Type> forceLoad(Arg arg) = 0;

        virtual bool clearCache(Arg arg) = 0;
        virtual void clearCache() = 0;
        virtual void pruneCache() = 0;
    };

    template<typename Type>
    using ICachedLoader = IBasicCachedLoader<Type, std::filesystem::path>;

    template<typename Type, typename Arg>
    class DARMOK_EXPORT BasicCachedLoader : public IBasicCachedLoader<Type, Arg>
    {
    public:
        using Resource = Type;

        std::shared_ptr<Resource> forceLoad(Arg arg) override
        {
            auto res = doLoad(arg);
            _cache[arg] = res;
            return res;
        }

        std::shared_ptr<Resource> operator()(Arg arg)
        {
            auto itr = _cache.find(arg);
            if (itr != _cache.end())
            {
                if (auto res = itr->second.lock())
                {
                    return res;
                }
            }
            return forceLoad(arg);
        }

        bool clearCache(Arg arg) override
        {
            auto itr = _cache.find(arg);
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
        virtual std::shared_ptr<Resource> doLoad(Arg arg) = 0;
    private:
        std::unordered_map<Arg, std::weak_ptr<Resource>> _cache;
    };

    template<typename Type>
    using CachedLoader = BasicCachedLoader<Type, std::filesystem::path>;

    template<typename Type, typename DefinitionType, typename Arg>
    class DARMOK_EXPORT BX_NO_VTABLE IBasicFromDefinitionLoader : public IBasicCachedLoader<Type, Arg>
    {
    public:
        using Resource = Type;
        using Definition = DefinitionType;
        virtual std::shared_ptr<Definition> getDefinition(const std::shared_ptr<Resource>& res) = 0;
        virtual std::shared_ptr<Resource> getResource(const std::shared_ptr<Definition>& def) = 0;
        virtual std::shared_ptr<Resource> loadResource(const std::shared_ptr<Definition>& def, bool force = false) = 0;
        virtual std::shared_ptr<Definition> loadDefinition(Arg arg, bool force = false) = 0;
    };

    template<typename Type, typename DefinitionType>
    using IFromDefinitionLoader = IBasicFromDefinitionLoader<Type, DefinitionType, std::filesystem::path>;

    template<typename Interface, typename DefinitionLoader, typename Arg>
    class DARMOK_EXPORT BasicFromDefinitionLoader : public Interface
    {
    public:
        using Resource = Interface::Resource;
        using Definition = DefinitionLoader::Resource;

        BasicFromDefinitionLoader(DefinitionLoader& defLoader) noexcept
            : _defLoader(defLoader)
        {
        }

        std::shared_ptr<Resource> operator()(Arg arg)
        {
            auto def = loadDefinition(arg);
            return loadResource(def);
        }

        std::shared_ptr<Resource> forceLoad(Arg arg)
        {
            auto def = loadDefinition(arg, true);
            return loadResource(def, true);
        }

        bool isCached(const std::shared_ptr<Definition>& def) const noexcept
        {
            auto itr = _resCache.find(def);
            return itr != _resCache.end();
        }

        bool isCached(Arg arg) const noexcept
        {
            auto itr = _defCache.find(arg);
            return itr != _defCache.end();
        }

        bool isCached(const std::shared_ptr<Resource>& res) const noexcept
        {
            auto itr = std::find_if(_resCache.begin(), _resCache.end(),
                [res](auto& elm) { return elm.second.lock() == res; });
            return itr != _resCache.end();
        }

        std::shared_ptr<Definition> loadDefinition(Arg arg, bool force = false)
        {
            if (!force)
            {
                auto itr = _defCache.find(arg);
                if (itr != _defCache.end())
                {
                    return itr->second;
                }
            }
            if (auto def = _defLoader(arg))
            {
                _defCache[arg] = def;
                return def;
            }
            return nullptr;
        }

        std::shared_ptr<Resource> getResource(const std::shared_ptr<Definition>& def)
        {
            if (!def)
            {
                return nullptr;
            }
            auto itr = _resCache.find(def);
            if (itr != _resCache.end())
            {
                if (auto res = itr->second.lock())
                {
                    return res;
                }
            }
            return nullptr;
        }

        std::shared_ptr<Resource> loadResource(const std::shared_ptr<Definition>& def, bool force = false)
        {
            if (!def)
            {
                return nullptr;
            }
            if (!force)
            {
                if (auto res = getResource(def))
                {
                    return res;
                }
            }
            auto res = create(def);
            _resCache[def] = res;
            return res;
        }

        std::shared_ptr<Definition> getDefinition(const std::shared_ptr<Resource>& res) noexcept
        {
            if (!res)
            {
                return nullptr;
            }
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

        bool clearCache(Arg arg) noexcept
        {
            auto itr = _defCache.find(arg);
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
            // TODO: check for static Resource::create methods
            return nullptr;
        }
    private:
        DefinitionLoader& _defLoader;
        std::unordered_map<Arg, std::shared_ptr<Definition>> _defCache;
        std::unordered_map<std::shared_ptr<Definition>, std::weak_ptr<Resource>> _resCache;
    };

    template<typename Type, typename DefinitionLoader>
    using FromDefinitionLoader = BasicFromDefinitionLoader<Type, DefinitionLoader, std::filesystem::path>;

    template<typename Interface>
    class DARMOK_EXPORT CerealLoader final : public Interface
    {
    public:
        using Resource = Interface::Resource;

        CerealLoader(IDataLoader& dataLoader) noexcept
            : _dataLoader(dataLoader)
        {
        }

        std::shared_ptr<Resource> operator()(std::filesystem::path path) override
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