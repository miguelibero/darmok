#pragma once

namespace darmok
{
    enum class RigidBody3dMotionType
    {
        Static,
        Dynamic,
        Kinematic
    };

    enum class Physics3dBackFaceMode : uint8_t
    {
        IgnoreBackFaces,
        CollideWithBackFaces,
    };
}