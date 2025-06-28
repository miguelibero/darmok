#pragma once

#include <darmok/scene_fwd.hpp>

#include <variant>
#include <string>

namespace darmok::editor
{
    struct AssetHandler
    {
        uint32_t type;
        std::string path;
    };

    using SelectableObject = std::variant<
        Entity,
        AssetHandler
    >;
}