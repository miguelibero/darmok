#include <darmok/culling.hpp>

namespace darmok
{
    void OcclusionCuller::init(Camera& cam, Scene& scene, App& app) noexcept
    {

    }

    void OcclusionCuller::shutdown() noexcept
    {

    }

    bool OcclusionCuller::shouldEntityBeCulled(Entity entity) noexcept
    {
        return false;
    }

    void FrustrumCuller::init(Camera& cam, Scene& scene, App& app) noexcept
    {

    }

    void FrustrumCuller::shutdown() noexcept
    {

    }

    bool FrustrumCuller::shouldEntityBeCulled(Entity entity) noexcept
    {
        return false;
    }
}