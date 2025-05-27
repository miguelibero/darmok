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
    class Transform;
    class Camera;
    class Renderable;
    class Skinnable;
    
    class SceneArchive final : public IComponentLoadContext
    {
        Scene& _scene;
        const protobuf::Scene& _sceneDef;
		AssetPack _assetPack;
        size_t _count = 0;
        entt::id_type _type;
        entt::id_type _entityOffset;
        std::optional<std::string> _error;
		std::vector<std::function<expected<void, std::string>()>> _loadFunctions;

        // IComponentLoadContext
        AssetPack& getAssets() override;
        Entity getEntity(uint32_t id) const override;
        const Scene& getScene() const override;
        Scene& getScene() override;

    public:
        SceneArchive(const protobuf::Scene& sceneDef, Scene& scene);

        entt::continuous_loader createLoader();

        void operator()(std::underlying_type_t<Entity>& count);

        using Components = std::variant<Transform, Renderable, Skinnable>;

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

        template<std::size_t I = 0>
        expected<void, std::string> loadComponents(entt::continuous_loader& loader)
        {
            auto result = load<std::variant_alternative_t<I, Components>>(loader);
            if (!result)
            {
                return unexpected{ result.error() };
            }
            if constexpr (I + 1 < std::variant_size_v<Components>)
            {
                return loadComponents<I + 1>(loader);
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
				_loadFunctions.push_back(std::move(func));
                ++_count;
            }
        }
    };

    class Scene;
    class SceneImporterImpl final
    {
    public:
        using Error = std::string;
        using Definition = protobuf::Scene;
        using Result = expected<Entity, Error>;
        Result operator()(Scene& scene, const Definition& def);
    };
}