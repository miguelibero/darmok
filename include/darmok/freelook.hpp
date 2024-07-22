#pragma once

#include <optional>
#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/scene.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/input.hpp>
#include <darmok/window_fwd.hpp>

namespace darmok
{
    class Window;
    class Camera;

    struct DARMOK_EXPORT FreelookConfig final
    {
        float mouseSensitivity = 5.F;
        float keyboardSensitivity = 10.F;
        glm::vec2 maxMouseAngle = glm::vec2(30, 30);
        std::shared_ptr<IInputEvent> inputEvent = std::make_shared<KeyboardInputEvent>( KeyboardKey::F4 );
    };
    
    class DARMOK_EXPORT FreelookController final : public ISceneComponent, public IInputEventListener
    {
    public:
        using Config = FreelookConfig;
        FreelookController(Camera& cam, const Config& config = {}) noexcept;
        void init(Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void update(float deltaTime) noexcept override;
        void onInputEvent(const IInputEvent& ev) noexcept override;
        FreelookController& setEnabled(bool enabled) noexcept;
        bool isEnabled() const noexcept;
    private:
        OptionalRef<Input> _input;
        OptionalRef<Window> _win;
        Camera& _cam;
        WindowCursorMode _winCursorMode;
        glm::quat _rot;
        glm::vec3 _pos;
        glm::vec3 _scale;
        bool _enabled;
        Config _config;
    };
}