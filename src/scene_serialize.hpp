#pragma once

#include <darmok/protobuf/scene.pb.h>
#include <darmok/expected.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/asset_pack.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/optional_ref.hpp>

#include <string>
#include <optional>
#include <variant>
#include <functional>

namespace darmok
{
    class SceneArchive final : public IComponentLoadContext
    {
        entt::continuous_loader _loader;
        Scene& _scene;
        AssetPackConfig _assetConfig;

        OptionalRef<const protobuf::Scene> _sceneDef;
        std::optional<AssetPack> _assetPack;
        size_t _count = 0;
        entt::id_type _type;
        std::optional<std::string> _error;

        using Result = expected<void, std::string>;
        using LoadFunction = std::function<Result()>;
		std::vector<LoadFunction> _loadFuncs;
        using ComponentLoadFunction = std::function<Result()>;
        std::vector<ComponentLoadFunction> _postLoadFuncs;

        // IComponentLoadContext
        IAssetContext& getAssets() override;
        const Scene& getScene() const override;
        Scene& getScene() override;

        template<typename T>
        expected<void, std::string> loadComponent()
        {
            using Def = typename T::Definition;
            _type = protobuf::getTypeId(*Def::descriptor());
            _loader.get<T>(*this);
            if (_error)
            {
                return unexpected{ *_error };
            }
            return {};
        }

        expected<void, std::string> loadComponents()
        {
            for (auto& func : _loadFuncs)
            {
                auto result = func();
                if (!result)
                {
                    return unexpected{ result.error() };
                }
            }
            return {};
        }

        std::pair<uint32_t, const google::protobuf::Any*> getComponentData();

    public:
        SceneArchive(Scene& scene, const AssetPackConfig& assetConfig);

        // IComponentLoadContext
        Entity getEntity(uint32_t id) const override;

        AssetPack& getAssetPack();

        template<typename T>
        void registerComponent()
        {
            auto func = [this]() -> expected<void, std::string>
            {
                return loadComponent<T>();
            };
            _loadFuncs.push_back(std::move(func));
        }

        expected<Entity, std::string> load(const protobuf::Scene& sceneDef);

        void operator()(std::underlying_type_t<Entity>& count);       

        template<typename T>
        void operator()(T& obj)
        {
            if(!_sceneDef)
            {
                _error = "Scene definition not set";
                return;
			}
            if constexpr (std::is_same_v<T, Entity>)
            {
                if (_type == 0)
                {
                    ++_count;
                    obj = static_cast<Entity>(_count);
                }
                else
                {
                    auto elm = getComponentData();
					obj = getEntity(elm.first);
                }
            }
            else
            {
				auto elm = getComponentData();
                if (!elm.second)
                {
                    return;
                }
                typename T::Definition def;
                if (!elm.second->UnpackTo(&def))
                {
                    _error = "Could not unpack component";
                    return;
                }

                // we need to delay the load() calls to guarantee that
                // all the components are present before
                auto entity = getEntity(elm.first);
                auto func = [this, entity, def]() -> expected<void, std::string>
                {
                    if (auto comp = _scene.getComponent<T>(entity))
                    {
                        return comp->load(def, *this);
                    }
                    return {};
                };
                _postLoadFuncs.push_back(std::move(func));

                ++_count;
            }
        }
    };

    class Scene;
    class SceneImporterImpl final
    {
    public:
        SceneImporterImpl(Scene& scene, const AssetPackConfig& assetConfig);
        using Error = std::string;
        using Definition = protobuf::Scene;
        using Result = expected<Entity, Error>;
        Result operator()(const Scene::Definition& def);

        IComponentLoadContext& getComponentLoadContext() noexcept;
        AssetPack& getAssetPack() noexcept;
    private:
		SceneArchive _archive;
    };

    class SceneLoaderImpl final
    {
    public:
        SceneLoaderImpl(ISceneDefinitionLoader& defLoader, const AssetPackConfig& assetConfig);
        expected<Entity, std::string> operator()(Scene& scene, std::filesystem::path path);
    private:
        ISceneDefinitionLoader& _defLoader;
        AssetPackConfig _assetConfig;
    };
}