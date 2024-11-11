#pragma once

#include <unordered_set>

namespace darmok
{
    class Transform;
    using TransformChildren = std::unordered_set<std::reference_wrapper<Transform>>;
}