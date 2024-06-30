#pragma once

#include <memory>
#include <darmok/export.h>
#include <darmok/app.hpp>

namespace darmok
{
    class Camera;
    class Program;
}

namespace darmok::physics3d
{
    class PhysicsDebugRendererImpl;
    class PhysicsSystem;

    class DARMOK_EXPORT PhysicsDebugRenderer : public AppComponent
    {
    public:
        PhysicsDebugRenderer(PhysicsSystem& system, const Camera& cam, const std::shared_ptr<Program>& prog = nullptr) noexcept;
        ~PhysicsDebugRenderer() noexcept;
        void init(App& app) override;
        bgfx::ViewId render(bgfx::ViewId viewId) const override;
    private:
        std::unique_ptr<PhysicsDebugRendererImpl> _impl;
    };
}