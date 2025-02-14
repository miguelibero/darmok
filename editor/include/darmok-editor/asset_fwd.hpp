#pragma once

#include <darmok/scene_fwd.hpp>
#include <darmok/program_fwd.hpp>

#include <variant>
#include <memory>
#include <string>

namespace darmok
{
    struct ProgramSource;
    struct MeshSource;
    class Material;
    class Scene;
    struct TextureDefinition;
    struct Model;
}

namespace darmok::editor
{
    using TextureAsset = std::shared_ptr<TextureDefinition>;
    using ProgramAsset = std::variant<StandardProgramType, std::shared_ptr<ProgramSource>>;
    using MaterialAsset = std::shared_ptr<Material>;
    using MeshAsset = std::shared_ptr<MeshSource>;
    using ModelAsset = std::shared_ptr<Model>;
    using SceneAsset = std::shared_ptr<Scene>;
    using SelectableObject = std::variant<
        Entity,
        SceneAsset,
        TextureAsset,
        ProgramAsset,
        MaterialAsset,
        MeshAsset,
        ModelAsset
    >;
}