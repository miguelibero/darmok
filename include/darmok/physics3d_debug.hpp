#pragma once

#include <memory>
#include <darmok/export.h>
#include <darmok/camera.hpp>

namespace darmok
{
    class Camera;
    class Program;
}

namespace darmok::physics3d
{
    class PhysicsDebugRendererImpl;
    class PhysicsSystem;

    class DARMOK_EXPORT PhysicsDebugRenderer : public ICameraComponent
    {
    public:
        PhysicsDebugRenderer(PhysicsSystem& system, const std::shared_ptr<Program>& prog = nullptr) noexcept;
        ~PhysicsDebugRenderer() noexcept;
        void init(Camera& cam, Scene& scene, App& app) override;
        void shutdown() override;
        bgfx::ViewId afterRender(bgfx::ViewId viewId) override;
        bool isEnabled() const noexcept;
        PhysicsDebugRenderer& setEnabled(bool enabled) noexcept;
    private:
        std::unique_ptr<PhysicsDebugRendererImpl> _impl;
    };
}