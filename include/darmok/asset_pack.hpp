#pragma once

#include <darmok/export.h>
#include <darmok/scene.hpp>
#include <darmok/program_core.hpp>
#include <darmok/material.hpp>
#include <darmok/texture.hpp>
#include <darmok/model.hpp>

#include <cereal/cereal.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/unordered_set.hpp>

namespace darmok
{
    struct DARMOK_EXPORT AssetPack final
    {
        template<typename Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(programs),
                CEREAL_NVP(textures),
                CEREAL_NVP(materials),
                CEREAL_NVP(models),
                CEREAL_NVP(scenes)
            );
        }
        std::unordered_set<std::shared_ptr<ProgramDefinition>> programs;
        std::unordered_set<std::shared_ptr<TextureDefinition>> textures;
        std::unordered_set<std::shared_ptr<Material>> materials;
        std::unordered_set<std::shared_ptr<Model>> models;
        std::unordered_set<std::shared_ptr<Scene>> scenes;
        // TODO: skeleton & animations

        void clear() noexcept;
    };
}