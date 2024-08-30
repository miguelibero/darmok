#pragma once

#ifdef _DEBUG

#include <memory>
#include <optional>
#include <unordered_set>

#include <darmok/export.h>
#include <darmok/render_scene.hpp>
#include <darmok/input.hpp>

namespace darmok
{
    class Camera;
    class IFont;
    class Program;
    using ProgramDefines = std::unordered_set<std::string>;
}

namespace darmok::physics3d
{
    class PhysicsDebugCameraComponentImpl;
    class PhysicsSystem;

    struct DARMOK_EXPORT PhysicsDebugConfig final
    {
        std::optional<InputEvent> enableEvent = KeyboardInputEvent{ KeyboardKey::F8 };
        float alpha = 0.3F;
        std::shared_ptr<Program> program;
        ProgramDefines programDefines;
    };

    class DARMOK_EXPORT PhysicsDebugCameraComponent: public ICameraComponent
    {
    public:
        using Config = PhysicsDebugConfig;
        PhysicsDebugCameraComponent(const Config& = {}) noexcept;
        ~PhysicsDebugCameraComponent() noexcept;
        void init(Camera& cam, Scene& scene, App& app) override;
        void shutdown() override;
        void beforeRenderView(IRenderGraphContext& context) override;

        bool isEnabled() const noexcept;
        PhysicsDebugCameraComponent& setEnabled(bool enabled) noexcept;
        PhysicsDebugCameraComponent& setFont(const std::shared_ptr<IFont>& font) noexcept;
    private:
        std::unique_ptr<PhysicsDebugCameraComponentImpl> _impl;
    };
}

#endif