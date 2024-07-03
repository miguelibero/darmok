#pragma once

#include <memory>
#include <optional>
#include <darmok/export.h>
#include <darmok/camera.hpp>
#include <darmok/input.hpp>

namespace darmok
{
    class Camera;
    class Material;
}

namespace darmok::physics3d
{
    class PhysicsDebugRendererImpl;
    class PhysicsSystem;

    struct DARMOK_EXPORT PhysicsDebugConfig final
    {
        std::shared_ptr<Material> material;
        std::optional<InputBindingKey> bindingKey = KeyboardBindingKey{ KeyboardKey::F7 };
    };

    class DARMOK_EXPORT PhysicsDebugRenderer : public ICameraComponent
    {
    public:
        using Config = PhysicsDebugConfig;
        PhysicsDebugRenderer(PhysicsSystem& system, const Config& = {}) noexcept;
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