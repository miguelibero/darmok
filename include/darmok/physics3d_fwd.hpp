#pragma once

#include <variant>

namespace darmok
{
    struct Cuboid;
    struct Sphere;
    struct Capsule;
}

namespace darmok::physics3d
{
    enum class PhysicsBodyMotionType
    {
        Static,
        Dynamic,
        Kinematic
    };

    enum class BackFaceMode : uint8_t
    {
        IgnoreBackFaces,
        CollideWithBackFaces,
    };

    using PhysicsShape = std::variant<Cuboid, Sphere, Capsule>;
}