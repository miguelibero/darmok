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
    struct SelectedAsset
    {
        uint32_t type;
        std::string path;
    };
    using SelectableObject = std::variant<
        Entity,
        SelectedAsset
    >;
}