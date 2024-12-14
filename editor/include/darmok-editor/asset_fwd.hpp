#pragma once

#include <darmok/scene_fwd.hpp>
#include <darmok/program_fwd.hpp>

#include <variant>
#include <memory>

namespace darmok
{
    struct ProgramSource;
    class Material;
}

namespace darmok::editor
{
    using ProgramAsset = std::variant<StandardProgramType, std::shared_ptr<ProgramSource>>;
    using MaterialAsset = std::shared_ptr<Material>;
    using SelectableObject = std::variant<Entity, MaterialAsset, ProgramAsset>;
}