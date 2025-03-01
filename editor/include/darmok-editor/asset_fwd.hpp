#pragma once

#include <darmok/scene_fwd.hpp>

#include <variant>
#include <memory>
#include <string>

namespace darmok::protobuf
{
    class ProgramSource;
    class MeshSource;
    class MaterialSource;
    class TextureSource;
}

namespace darmok::editor
{
    using TextureAsset = std::shared_ptr<protobuf::TextureSource>;
    using ProgramAsset = std::shared_ptr<protobuf::ProgramSource>;
    using MaterialAsset = std::shared_ptr<protobuf::MaterialSource>;
    using MeshAsset = std::shared_ptr<protobuf::MeshSource>;
    using SceneAsset = std::shared_ptr<Scene>;
    using SelectableObject = std::variant<
        Entity,
        SceneAsset,
        TextureAsset,
        ProgramAsset,
        MaterialAsset,
        MeshAsset
    >;
}