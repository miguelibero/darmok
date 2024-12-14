#pragma once

#include <darmok/export.h>
#include <darmok/scene.hpp>
#include <darmok/texture.hpp>
#include <darmok/mesh.hpp>
#include <darmok/model.hpp>

#include <cereal/cereal.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/unordered_set.hpp>

namespace darmok
{
    struct DARMOK_EXPORT AssetPack final
    {
        std::unordered_set<std::shared_ptr<TextureDefinition>> textures;
        std::unordered_set<std::shared_ptr<MeshDefinition>> meshes;
        std::unordered_set<std::shared_ptr<Model>> models;
        std::unordered_set<std::shared_ptr<Scene>> scenes;
        // TODO: skeleton & animations

        void clear() noexcept;

        template<typename Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(textures),
                CEREAL_NVP(meshes),
                CEREAL_NVP(models),
                CEREAL_NVP(scenes)
            );
        }
    };
}