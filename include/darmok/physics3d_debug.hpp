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
        void afterRenderView(bgfx::Encoder& encoder, bgfx::ViewId viewId) override;
    private:
        std::unique_ptr<PhysicsDebugRendererImpl> _impl;
    };
}