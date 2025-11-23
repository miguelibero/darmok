#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <darmok/string.hpp>
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
        [[nodiscard]] virtual Result operator()(Argument arg) noexcept = 0;
    };

    template<typename Type>
    using ILoader = IBasicLoader<Type, std::filesystem::path>;

    template<typename Interface>
    class DARMOK_EXPORT EmptyLoader final : public Interface
    {
    public:
        using Resource = Interface::Resource;
        using Result = Interface::Result;
        using Error = Interface::Error;

        Result operator()(std::filesystem::path path) noexcept override
        {
			return unexpected<Error>{ "empty" };
        }
    };

    template<typename Interface>
    class DARMOK_EXPORT MultiLoader : public Interface
    {
    public:
        using Resource = Interface::Resource;
        using Result = Interface::Result;
        using Error = Interface::Error;

        struct Element final
        {
            std::reference_wrapper<Interface> loader;
            std::vector<std::string> exts;
        };
        using Elements = std::vector<Element>;

    protected:
        Elements _loaders;

        std::vector<std::string> splitExtensions(const std::string& exts) const noexcept
        {
            auto vec = StringUtils::split(exts, ";");
            std::erase_if(vec, [](const std::string& ext) {
                return ext.empty();
            });
            return vec;
		}

        void addLoaders() noexcept
        {
        }

        template<typename T, typename... Rest>
        void addLoaders(T& first, Rest&... rest) noexcept
        {
            _loaders.emplace_back(first);
            addLoaders(rest...);
        }

        std::vector<std::reference_wrapper<Interface>> getLoaders(std::filesystem::path path) noexcept
        {
            std::vector<std::reference_wrapper<Interface>> loaders;
            auto ext = path.extension();
            for (auto& [loader, exts] : _loaders)
            {
                if (exts.empty())
                {
                    continue;
                }
                auto itr = std::find(exts.begin(), exts.end(), ext);
                if (itr == exts.end())
                {
                    continue;
                }
                loaders.push_back(loader);
            }
            for (auto& [loader, exts] : _loaders)
            {
                if (exts.empty())
                {
                    loaders.push_back(loader);
                }
            }
            return loaders;
        }

    public:

        template<typename... Args>
        MultiLoader(Args&... loaders) noexcept
        {
            _loaders.reserve(sizeof...(loaders));
            addLoaders(loaders...);
        }

        void addFront(Interface& loader, const std::string& exts = "") noexcept
        {
            _loaders.emplace(_loaders.begin(), loader, splitExtensions(exts));
        }

        void addBack(Interface& loader, const std::string& exts = "") noexcept
        {
            _loaders.emplace_back(loader, splitExtensions(exts));
        }

        Result operator()(std::filesystem::path path) noexcept override
        {
            std::vector<std::string> errors;
            for (auto loader : getLoaders(path))
            {
                auto result = loader.get()(path);
                if (result)
                {
                    return result;
                }
                errors.push_back(result.error());
            }

            return unexpected{ StringUtils::joinErrors(errors) };
        }
    };

    template<typename Interface>
    class DARMOK_EXPORT BX_NO_VTABLE ICachedLoader : public Interface
    {
    public:
		using Argument = Interface::Argument;
		using Result = Interface::Result;
        using Resource = Interface::Resource;

        virtual Result forceLoad(Argument arg) noexcept = 0;

        virtual bool releaseCache(Argument arg) noexcept = 0;
        virtual bool releaseResourceCache(const Resource& res) noexcept = 0;
        virtual void clearCache() noexcept = 0;
        virtual void pruneCache() noexcept = 0;

        virtual bool isCached(Argument arg) const noexcept = 0;
        virtual bool isResourceCached(const Resource& res) const noexcept = 0;
    };

    template<typename Interface>
    class DARMOK_EXPORT CachedLoader : public ICachedLoader<Interface>
    {
    public:
        using Resource = Interface::Resource;
        using Argument = Interface::Argument;
        std::shared_ptr<Resource> forceLoad(Argument arg) noexcept override
        {
            auto res = doLoad(arg);
            _cache[arg] = res;
            return res;
        }

        std::shared_ptr<Resource> operator()(Argument arg) noexcept
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

        bool releaseCache(Argument arg) noexcept override
        {
            auto itr = _cache.find(arg);
            if (itr == _cache.end())
            {
                return false;
            }
            _cache.erase(itr);
            return true;
        }

        bool releaseResourceCache(const Resource& res) noexcept override
        {
            auto itr = std::find_if(_cache.begin(), _cache.end(), [&res](auto& elm)
            {
                return elm.second.lock() == &res;
            });
            if (itr == _cache.end())
            {
                return false;
            }

            return true;
        }

        void clearCache() noexcept override
        {
            _cache.clear();
        }

        void pruneCache() noexcept override
        {
            // remove cache items that don't have any references
            for (auto itr = _cache.begin(); itr != _cache.end();)
            {
                if (itr->second.lock() == nullptr)
                {
                    itr = _cache.erase(itr);
                }
                else
                {
                    ++itr;
                }
            }
        }

        bool isCached(Argument arg) const noexcept override
        {
            auto itr = _cache.find(arg);
            return itr != _cache.end();
        }

        bool isResourceCached(const Resource& res) const noexcept override
        {
            for (auto itr = _cache.begin(); itr != _cache.end();)
            {
                if (itr->second.lock() == &res)
                {
                    return true;
                }
            }
            return false;
        }

    protected:
        virtual std::shared_ptr<Resource> doLoad(Argument arg) = 0;
    private:
        std::unordered_map<Argument, std::weak_ptr<Resource>> _cache;
    };

    template<typename Interface>
    class DARMOK_EXPORT MultiCachedLoader : public MultiLoader<Interface>
    {
    public:
        using Base = MultiLoader<Interface>;
        using Result = Interface::Result;
        using Error = Interface::Error;
        using Resource = Interface::Resource;

        template<typename... Args>
        MultiCachedLoader(Args&... loaders) noexcept
            : Base(loaders...)
        {
        }

        Result forceLoad(std::filesystem::path path) noexcept override
        {
            std::vector<std::string> errors;
            for (auto loader : Base::getLoaders(path))
            {
                auto result = loader.get().forceLoad(path);
                if (result)
                {
                    return result;
                }
                errors.push_back(result.error());
            }
            return unexpected<Error>{ StringUtils::joinErrors(errors) };
        }

        bool releaseCache(std::filesystem::path path) noexcept override
        {
            auto changed = false;
            for (auto& [loader, exts] : Base::_loaders)
            {
                if (loader.get().releaseCache(path))
                {
                    changed = true;
                }
            }
            return changed;
        }

        bool releaseResourceCache(const Resource& res) noexcept override
        {
            for (auto& [loader, exts] : Base::_loaders)
            {
                if (loader.get().releaseResourceCache(res))
                {
                    return true;
                }
            }
            return false;
        }

        void clearCache() noexcept override
        {
            for (auto& [loader, exts] : Base::_loaders)
            {
                loader.get().clearCache();
            }
        }

        void pruneCache() noexcept override
        {
            for (auto& [loader, exts] : Base::_loaders)
            {
                loader.get().pruneCache();
            }
        }

        bool isCached(std::filesystem::path path) const noexcept override
        {
            auto cached = false;
            for (auto& [loader, exts] : Base::_loaders)
            {
                if (loader.get().isCached(path))
                {
                    cached = true;
                }
            }
            return cached;
        }

        bool isResourceCached(const Resource& res) const noexcept override
        {
            auto cached = false;
            for (auto& [loader, exts] : Base::_loaders)
            {
                if (loader.get().isResourceCached(res))
                {
                    cached = true;
                }
            }
            return cached;
        }
    };

    template<typename Interface, typename DefinitionType>
    class DARMOK_EXPORT BX_NO_VTABLE IFromDefinitionLoader : public ICachedLoader<Interface>
    {
    public:
        using Resource = ICachedLoader<Interface>::Resource;
        using Argument = ICachedLoader<Interface>::Argument;
        using Result = ICachedLoader<Interface>::Result;
        using Error = ICachedLoader<Interface>::Error;
        using Definition = DefinitionType;
        using DefinitionResult = expected<std::shared_ptr<Definition>, std::string>;

        virtual std::shared_ptr<Definition> getDefinition(const Resource& res) const noexcept = 0;
        virtual std::shared_ptr<Resource> getResource(const Definition& def) const noexcept = 0;
        virtual Result loadResource(std::shared_ptr<Definition> def, bool force = false) noexcept = 0;
        virtual Result reloadResource(const Definition& def) noexcept = 0;
        virtual DefinitionResult reloadDefinition(const Definition& def) noexcept = 0;
        virtual DefinitionResult loadDefinition(Argument arg, bool force = false) noexcept = 0;
        virtual bool releaseDefinitionCache(const Definition& def) noexcept = 0;
        virtual bool isDefinitionCached(const Definition& def) const noexcept = 0;

        Result reload(Argument arg)
        {
			auto defResult = loadDefinition(arg);
            if (!defResult)
            {
                return unexpected<Error>{ defResult.error() };
            }
			return reloadResource(*defResult.value());
        }
    };

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
            : _defLoader{ defLoader }
        {
        }

        Result operator()(Argument arg) noexcept override
        {
            auto defResult = loadDefinition(arg);
            if (!defResult)
            {
                return unexpected<Error>{ defResult.error() };
            }
            return loadResource(defResult.value());
        }

        Result forceLoad(Argument arg) noexcept override
        {
            auto defResult = loadDefinition(arg, true);
            if (!defResult)
            {
                return unexpected<Error>{ defResult.error() };
            }
            return loadResource(defResult.value(), true);
        }

        bool isDefinitionCached(const Definition& def) const noexcept override
        {
            auto ptr = &def;
            auto itr = std::find_if(_resCache.begin(), _resCache.end(),
                [ptr](auto& elm) { return elm.first.get() == ptr; });
            return itr != _resCache.end();
        }

        bool isCached(Argument arg) const noexcept override
        {
            auto itr = _defCache.find(arg);
            return itr != _defCache.end();
        }

        bool isResourceCached(const Resource& res) const noexcept override
        {
            auto ptr = &res;
            auto itr = std::find_if(_resCache.begin(), _resCache.end(),
                [ptr](auto& elm) { return elm.second.lock().get() == ptr; });
            return itr != _resCache.end();
        }

        DefinitionResult loadDefinition(Argument arg, bool force = false) noexcept override
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

        std::shared_ptr<Resource> getResource(const Definition& def) const noexcept override
        {
            auto ptr = &def;
            auto itr = std::find_if(_resCache.begin(), _resCache.end(),
                [ptr](auto& elm) { return elm.first.get() == ptr; });
            if (itr != _resCache.end())
            {
                if (auto res = itr->second.lock())
                {
                    return res;
                }
            }
            return nullptr;
        }

        Result loadResource(std::shared_ptr<Definition> def, bool force = false) noexcept override
        {
            if (!force)
            {
                if (auto res = getResource(*def))
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

        DefinitionResult reloadDefinition(const Definition& def) noexcept override
        {
            auto ptr = &def;
            auto itr = std::find_if(_defCache.begin(), _defCache.end(),
                [ptr](auto& elm) { return elm.second.get() == ptr; });
            if (itr == _defCache.end())
            {
                return unexpected<Error>{ "definition not found in cache" };
            }
            return loadDefinition(itr->first, true);
        }

        Result reloadResource(const Definition& def) noexcept override
        {
            auto ptr = &def;
            auto itrRes = std::find_if(_resCache.begin(), _resCache.end(),
                [ptr](auto& elm) { return elm.first.get() == ptr; });
            if(itrRes == _resCache.end())
            {
                return nullptr;
			}
			auto res = itrRes->second.lock();
            if(!res)
            {
                return nullptr;
			}
            auto itrDef = std::find_if(_defCache.begin(), _defCache.end(),
                [ptr](auto& elm) { return elm.second.get() == ptr; });
            if (itrDef == _defCache.end())
            {
                return unexpected<Error>{ "definition not found in cache" };
            }
            auto arg = itrDef->first;
            auto defResult = _defLoader(arg);
            if (!defResult)
            {
                return unexpected<Error>{ defResult.error() };
            }
			auto newDef = defResult.value();
            auto result = load(*res, newDef);
            if(!result)
            {
                return unexpected<Error>{ result.error() };
			}
            _resCache.erase(itrRes);
			_resCache[newDef] = res;
            _defCache.erase(itrDef);
            _defCache[arg] = newDef;
            return res;
        }

        std::shared_ptr<Definition> getDefinition(const Resource& res) const noexcept override
        {
            auto ptr = &res;
            auto itr = std::find_if(_resCache.begin(), _resCache.end(),
                [ptr](auto& elm) {
                    return elm.second.lock().get() == ptr;
                });
            if (itr != _resCache.end())
            {
                return itr->first;
            }
            return nullptr;
        }

        bool releaseCache(Argument arg) noexcept override
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

        bool releaseResourceCache(const Resource& res) noexcept override
        {
            auto ptr = &res;
            auto itr = std::find_if(_resCache.begin(), _resCache.end(),
                [ptr](auto& elm) { return elm.second.lock().get() == ptr; });
            return itr != _resCache.end();
        }

        bool releaseDefinitionCache(const Definition& def) noexcept override
        {
			auto ptr = &def;
            auto itr = std::find_if(_defCache.begin(), _defCache.end(),
                [ptr](auto& elm) { return elm.second.get() == ptr; });
            auto found = itr != _defCache.end();
            if (found)
            {
                _defCache.erase(itr);
            }
            auto itr2 = std::find_if(_resCache.begin(), _resCache.end(),
				[ptr](auto& elm) { return elm.first.get() == ptr; });
            ;
            if (itr2 != _resCache.end())
            {
                found = true;
                _resCache.erase(itr2);
            }
            return found;
        }

        void clearCache() noexcept override
        {
            _defCache.clear();
            _resCache.clear();
        }

        void pruneCache() noexcept override
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

        virtual Result create(std::shared_ptr<Definition> def) noexcept
        {
            if (!def)
            {
                return unexpected<Error>{"empty definition"};
            }
            if constexpr (std::is_constructible_v<Resource, std::shared_ptr<Definition>>)
            {
                return std::make_shared<Resource>(def);
            }
            if constexpr (std::is_constructible_v<Resource, Definition>)
            {
                return std::make_shared<Resource>(*def);
            }
            return unexpected<Error>{"could not create resource"};
        }

        virtual expected<void, Error> load(Resource& res, std::shared_ptr<Definition> def) noexcept
        {
            if constexpr (std::is_move_assignable_v<Resource>)
            {
                auto result = create(def);
                if (!result)
                {
                    return unexpected<Error>{ result.error() };
                }
                if (result.value())
                {
                    res = std::move(*result.value());
                }
				return {};
            }
            if constexpr (std::is_copy_assignable_v<Resource>)
            {
                auto result = create(def);
                if (!result)
                {
                    return unexpected<Error>{ result.error() };
                }
                if (result.value())
                {
                    res = *result.value();
                }
                return {};
            }
            return unexpected<Error>{"could not load resource"};
        }

    private:
        DefinitionLoader& _defLoader;
        std::unordered_map<Argument, std::shared_ptr<Definition>> _defCache;
        std::unordered_map<std::shared_ptr<Definition>, std::weak_ptr<Resource>> _resCache;
    };


    template<typename Interface>
    class DARMOK_EXPORT MultiFromDefinitionLoader final : public MultiCachedLoader<Interface>
    {
    public:
        using Definition = Interface::Definition;
        using Resource = Interface::Resource;
        using Result = Interface::Result;
        using DefinitionResult = Interface::DefinitionResult;
        using Error = Interface::Error;
        using Base = MultiCachedLoader<Interface>;

        template<typename... Args>
        MultiFromDefinitionLoader(Args&... loaders) noexcept
            : Base(loaders...)
        {
        }

        bool isDefinitionCached(const Definition& def) const noexcept override
        {
            auto cached = false;
            for (auto& [loader, exts] : Base::_loaders)
            {
                if (loader.get().isDefinitionCached(def))
                {
                    cached = true;
                }
            }
            return cached;
        }

        std::shared_ptr<Definition> getDefinition(const Resource& res) const noexcept override
        {
            for (auto& [loader, exts] : Base::_loaders)
            {
                if (auto def = loader.get().getDefinition(res))
                {
                    return def;
                }
            }
            return nullptr;
        }

        std::shared_ptr<Resource> getResource(const Definition& def) const noexcept override
        {
            for (auto& [loader, exts] : Base::_loaders)
            {
                if (auto res = loader.get().getResource(def))
                {
                    return res;
                }
            }
            return nullptr;
        }

        Result loadResource(std::shared_ptr<Definition> def, bool force = false) noexcept override
        {
            for (auto& [loader, exts] : Base::_loaders)
            {
                if (auto res = loader.get().loadResource(def, force))
                {
                    return res;
                }
            }
            return unexpected<Error>{ "no loaders found" };
        }

        Result reloadResource(const Definition& def) noexcept override
        {
            for (auto& [loader, exts] : Base::_loaders)
            {
                if (auto res = loader.get().reloadResource(def))
                {
                    return res;
                }
            }
            return unexpected<Error>{ "no loaders found" };
        }

        DefinitionResult reloadDefinition(const Definition& def) noexcept override
        {
            for (auto& [loader, exts] : Base::_loaders)
            {
                if (auto res = loader.get().reloadDefinition(def))
                {
                    return res;
                }
            }
            return unexpected<Error>{ "no loaders found" };
        }

        DefinitionResult loadDefinition(std::filesystem::path path, bool force = false) noexcept override
        {
            std::vector<std::string> errors;
            for (auto loader : Base::getLoaders(path))
            {
                auto result = loader.get().loadDefinition(path);
                if (result)
                {
                    return result;
                }
                errors.push_back(result.error());
            }
            return unexpected{ StringUtils::joinErrors(errors) };
        }

        bool releaseDefinitionCache(const Definition& def) noexcept override
        {
            for (auto& [loader, exts] : Base::_loaders)
            {
                if (loader.get().releaseDefinitionCache(def))
                {
                    return true;
                }
            }
            return false;
        }
    };
}