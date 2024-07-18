#pragma once

#include <darmok/export.h>
#include <darmok/render.hpp>

namespace darmok
{
    class Camera;

    class DARMOK_EXPORT DeferredRenderer final : public IRenderer
    {
    public:
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
    };
}