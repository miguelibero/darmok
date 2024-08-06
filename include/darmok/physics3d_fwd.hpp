#pragma once

#include <variant>

namespace darmok
{
    struct Cube;
    struct Sphere;
    struct Capsule;
    struct BoundingBox;
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

    using PhysicsShape = std::variant<Cube, Sphere, Capsule, Polygon, BoundingBox>;
}