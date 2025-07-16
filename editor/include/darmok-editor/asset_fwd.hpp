#pragma once

#include <darmok/scene_fwd.hpp>

#include <variant>
#include <filesystem>

namespace darmok::editor
{
    using SelectableObject = std::variant<
        Entity,
        std::filesystem::path // asset path
    >;
}