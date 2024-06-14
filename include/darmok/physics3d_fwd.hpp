#pragma once

namespace darmok::physics3d
{
    enum class RigidBodyMotionType
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
}