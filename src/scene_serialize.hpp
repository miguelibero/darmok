#pragma once

#include <darmok/protobuf/scene.pb.h>
#include <darmok/expected.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/asset_pack.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/scene_serialize.hpp>

#include <string>
#include <optional>
#include <variant>
#include <functional>

namespace darmok
{
    class SceneArchive final : public IComponentLoadContext
    {
        Scene& _scene;
        const protobuf::Scene& _sceneDef;
		AssetPack _assetPack;
        size_t _count = 0;
        entt::id_type _type;
        entt::id_type _entityOffset;
        std::optional<std::string> _error;

        using Result = expected<void, std::string>;
        using LoadFunction = std::function<Result(entt::continuous_loader& loader)>;
		std::vector<LoadFunction> _loadFunctions;
        using ComponentLoadFunction = std::function<Result()>;
        std::vector<ComponentLoadFunction> _compLoadFunctions;

        // IComponentLoadContext
        AssetPack& getAssets() override;
        Entity getEntity(uint32_t id) const override;
        const Scene& getScene() const override;
        Scene& getScene() override;

    public:
        SceneArchive(const protobuf::Scene& sceneDef, Scene& scene, const AssetPackFallbacks& assetPackFallbacks = {});

        entt::continuous_loader createLoader();

        void operator()(std::underlying_type_t<Entity>& count);
        
        template<typename T>
        void registerComponent()
        {
            auto func = [this](entt::continuous_loader& loader) -> expected<void, std::string>
            {
                return load<T>(loader);
            };
            _loadFunctions.push_back(std::move(func));
        }

        template<typename T>
        expected<void, std::string> load(entt::continuous_loader& loader)
        {
            using Def = typename T::Definition;
            _type = protobuf::getTypeId(*Def::descriptor());
            loader.get<T>(*this);
            if (_error)
            {
                return unexpected{ *_error };
            }
            return {};
        }

        expected<void, std::string> loadComponents(entt::continuous_loader& loader)
        {
            for(auto& func : _loadFunctions)
            {
                auto result = func(loader);
                if (!result)
                {
                    return unexpected{ result.error() };
                }
			}
            return {};
        }

        expected<Entity, std::string> finishLoad();

        template<typename T>
        void operator()(T& obj)
        {
            auto getComponentData = [this]() -> std::pair<uint32_t, const google::protobuf::Any*>
            {
                auto& typeComps = _sceneDef.registry().components();
                auto itr = typeComps.find(_type);
                if (itr == typeComps.end())
                {
                    _error = "Could not find component type";
                    return { 0, nullptr };
                }
                auto& comps = itr->second.components();
                auto itr2 = comps.begin();
                std::advance(itr2, _count);
                auto& key = itr2->first;
                if (itr2 == comps.end())
                {
                    _error = "Could not find component";
                    return {0, nullptr};
                }
                return { itr2->first, &itr2->second };
            };

            if constexpr (std::is_same_v<T, Entity>)
            {
                if (_type == 0)
                {
                    obj = getEntity(_count);
                    ++_count;
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
				auto entity = getEntity(elm.first);
                auto func = [this, entity, def]() -> expected<void, std::string>
                {
                    if (auto comp = _scene.getComponent<T>(entity))
                    {
                        return comp->load(def, *this);
                    }
                    return {};
                };
				_compLoadFunctions.push_back(std::move(func));
                ++_count;
            }
        }
    };

    class Scene;
    class SceneImporterImpl final
    {
    public:
        SceneImporterImpl(const AssetPackFallbacks& assetPackFallbacks = {});
        using Error = std::string;
        using Definition = protobuf::Scene;
        using Result = expected<Entity, Error>;
        Result operator()(Scene& scene, const Definition& def);
    private:
        AssetPackFallbacks _assetPackFallbacks;
    };
}