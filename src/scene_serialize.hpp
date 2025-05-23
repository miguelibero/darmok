#pragma once

#include <darmok/protobuf/scene.pb.h>
#include <darmok/expected.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/asset_pack.hpp>
#include <darmok/scene_fwd.hpp>

#include <string>
#include <optional>

namespace darmok
{
    class Transform;
    
    class SceneArchive final
    {
        const protobuf::Scene& _scene;
		AssetPack _assetPack;
        size_t _count = 0;
        entt::id_type _type;
        std::optional<std::string> _error;

    public:
        SceneArchive(const protobuf::Scene& scene);

        void operator()(std::underlying_type_t<Entity>& count);

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

        template<typename T>
        void operator()(T& obj)
        {
            if (_error)
            {
                return;
            }
            using CompMap = google::protobuf::Map<uint32_t, google::protobuf::Any>;
            auto getComponentData = [this]() -> CompMap::const_iterator
            {
                auto& typeComps = _scene.registry().components();
                auto itr = typeComps.find(_type);
                if (itr == typeComps.end())
                {
                    _error = "Could not find component type";
                    return {};
                }
                auto& comps = itr->second.components();
                auto itr2 = comps.begin();
                std::advance(itr2, _count);
                if (itr2 == comps.end())
                {
                    _error = "Could not find component";
                    return {};
                }
                return itr2;
            };

            if constexpr (std::is_same_v<T, Entity>)
            {
                if (_type == 0)
                {
                    obj = static_cast<Entity>(_count + 1);
                    ++_count;
                }
                else
                {
                    auto itr = getComponentData();
                    if (_error)
                    {
                        return;
                    }
					obj = static_cast<Entity>(itr->first);
                }
            }
            else
            {
				auto itr = getComponentData();
                if (_error)
                {
                    return;
                }
                typename T::Definition def;
                if (!itr->second.UnpackTo(&def))
                {
                    _error = "Could not unpack component";
                    return;
                }
                auto result = obj.load(def, _assetPack);
                if(!result)
                {
                    _error = result.error();
				}
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
        using Result = expected<void, Error>;
        Result operator()(Scene& scene, const Definition& def);
    };
}