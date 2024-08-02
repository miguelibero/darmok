#pragma once

#ifdef _DEBUG

#include <memory>
#include <optional>
#include <darmok/export.h>
#include <darmok/render.hpp>
#include <darmok/input.hpp>

namespace darmok
{
    class Camera;
    class Material;
    class IFont;
}

namespace darmok::physics3d
{
    class PhysicsDebugRendererImpl;
    class PhysicsSystem;

    struct DARMOK_EXPORT PhysicsDebugConfig final
    {
        std::shared_ptr<Material> material;
        std::optional<InputEvent> enableEvent = KeyboardInputEvent{ KeyboardKey::F8 };
        float alpha = 0.3F;
    };

    class DARMOK_EXPORT PhysicsDebugRenderer : public IRenderer
    {
    public:
        using Config = PhysicsDebugConfig;
        PhysicsDebugRenderer(PhysicsSystem& system, const Config& = {}) noexcept;
        ~PhysicsDebugRenderer() noexcept;
        void init(Camera& cam, Scene& scene, App& app) override;
        void shutdown() override;
        void renderReset() override;

        bool isEnabled() const noexcept;
        PhysicsDebugRenderer& setEnabled(bool enabled) noexcept;
        PhysicsDebugRenderer& setFont(const std::shared_ptr<IFont>& font) noexcept;
    private:
        std::unique_ptr<PhysicsDebugRendererImpl> _impl;
    };
}

#endif