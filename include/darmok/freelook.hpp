#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/scene.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/input.hpp>
#include <darmok/window_fwd.hpp>

namespace darmok
{
    class Transform;
    class Window;

    struct DARMOK_EXPORT FreelookConfig final
    {
        float mouseSensitivity = 5.F;
        float keyboardSensitivity = 10.F;
        glm::vec2 maxMouseAngle = glm::vec2(30, 30);
        std::optional<InputBindingKey> bindingKey = KeyboardBindingKey{ KeyboardKey::F4 };
    };
    
    class DARMOK_EXPORT FreelookController final : public ISceneComponent
    {
    public:
        using Config = FreelookConfig;
        FreelookController(Transform& trans, const Config& config = {});
        void init(Scene& scene, App& app) override;
        void shutdown() override;
        void update(float deltaTime) override;
        FreelookController& setEnabled(bool enabled) noexcept;
        bool isEnabled() const noexcept;
    private:

        void onBindingTriggered() noexcept;

        static const std::string _bindingsName;
        OptionalRef<Transform> _trans;
        OptionalRef<Input> _input;
        OptionalRef<Window> _win;
        WindowCursorMode _winCursorMode;
        bool _enabled;
        Config _config;

    };
}