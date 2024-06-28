#pragma once

namespace darmok::physics3d
{
    enum class GroundState
    {
        Grounded,
        GroundedSteep,
        NotSupported,
        Air,
    };
}