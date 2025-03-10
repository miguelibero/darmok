#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <darmok/string.hpp>
#include <darmok/serialize.hpp>
#include <darmok/data.hpp>
#include <darmok/expected.hpp>

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
        using Error = std::string;
        using Result = expected<std::shared_ptr<Resource>, Error>;
        using Argument = Arg;
        virtual ~IBasicLoader() = default;
        [[nodiscard]] virtual Result operator()(Argument arg) = 0;
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
    public:
        using Result = IBasicLoader<Type, Arg>::Result;
        virtual Result forceLoad(Arg arg) = 0;

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
        using Result = IBasicCachedLoader<Type, Arg>::Result;
        using DefinitionResult = expected<std::shared_ptr<Definition>, std::string>;

        virtual std::shared_ptr<Definition> getDefinition(const std::shared_ptr<Resource>& res) = 0;
        virtual std::shared_ptr<Resource> getResource(std::size_t hash) = 0;
        virtual std::shared_ptr<Resource> getResource(const std::shared_ptr<Definition>& def) = 0;
        virtual Result loadResource(const std::shared_ptr<Definition>& def, bool force = false) = 0;
        virtual DefinitionResult loadDefinition(Arg arg, bool force = false) = 0;
        virtual bool clearCache(const std::shared_ptr<Definition>& def) = 0;

        std::shared_ptr<Resource> getResource(const Definition& def)
        {
            return getResource(std::hash<Resource>{}(def));
        }
    };

    template<typename Type, typename DefinitionType>
    using IFromDefinitionLoader = IBasicFromDefinitionLoader<Type, DefinitionType, std::filesystem::path>;

    template<typename Interface, typename DefinitionLoader>
    class DARMOK_EXPORT FromDefinitionLoader : public Interface
    {
    public:
        using Result = Interface::Result;
        using Argument = Interface::Argument;
        using Resource = Interface::Resource;
        using Error = Interface::Error;
        using Definition = DefinitionLoader::Resource;
        using DefinitionResult = DefinitionLoader::Result;

        FromDefinitionLoader(DefinitionLoader& defLoader) noexcept
            : _defLoader(defLoader)
        {
        }

        FromDefinitionLoader(const FromDefinitionLoader& other) = delete;
        FromDefinitionLoader(FromDefinitionLoader&& other) = delete;

        Result operator()(Argument arg) override
        {
            auto defResult = loadDefinition(arg);
            if (!defResult)
            {
                return unexpected<Error>{ defResult.error() };
            }
            return loadResource(defResult.value());
        }

        Result forceLoad(Argument arg) override
        {
            auto defResult = loadDefinition(arg, true);
            if (!defResult)
            {
                return unexpected<Error>{ defResult.error() };
            }
            return loadResource(defResult.value(), true);
        }

        bool isCached(const std::shared_ptr<Definition>& def) const noexcept
        {
            auto itr = _resCache.find(def);
            return itr != _resCache.end();
        }

        bool isCached(Argument arg) const noexcept
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

        DefinitionResult loadDefinition(Argument arg, bool force = false) override
        {
            if (!force)
            {
                auto itr = _defCache.find(arg);
                if (itr != _defCache.end())
                {
                    return itr->second;
                }
            }
            auto defResult = _defLoader(arg);
            if (defResult)
            {
                _defCache[arg] = defResult.value();
            }
            return defResult;
        }

        std::shared_ptr<Resource> getResource(const std::shared_ptr<Definition>& def) override
        {
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

        Result loadResource(const std::shared_ptr<Definition>& def, bool force = false) override
        {
            if (!force)
            {
                if (auto res = getResource(def))
                {
                    return res;
                }
            }
            auto result = create(def);
            if (result)
            {
                _resCache[def] = result.value();
            }
            return result;
        }

        std::shared_ptr<Definition> getDefinition(const std::shared_ptr<Resource>& res) noexcept override
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

        bool clearCache(Argument arg) noexcept
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

        bool clearCache(const std::shared_ptr<Definition>& def) override
        {
            if (!def)
            {
                return false;
            }
            auto itr = std::find_if(_defCache.begin(), _defCache.end(),
                [&def](auto& elm) { return elm.second == def; });
            auto found = itr != _defCache.end();
            if (found)
            {
                _defCache.erase(itr);
            }
            auto size = _resCache.erase(def);
            return found || size > 0;
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
                else
                {
                    ++itr;
                }
            }
            for (auto itr = _defCache.begin(); itr != _defCache.end();)
            {
                if (!_resCache.contains(itr->second))
                {
                    itr = _defCache.erase(itr);
                }
                else
                {
                    ++itr;
                }
            }
        }

    protected:
        virtual Result create(const std::shared_ptr<Definition>& def)
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
            return unexpected<Error>{"could not create resource"};
        }
    private:
        DefinitionLoader& _defLoader;
        std::unordered_map<Argument, std::shared_ptr<Definition>> _defCache;
        std::unordered_map<std::shared_ptr<Definition>, std::weak_ptr<Resource>> _resCache;
    };

}