#pragma once

#include <optional>
#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/scene.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/input.hpp>
#include <darmok/window_fwd.hpp>
#include <darmok/collection.hpp>

namespace darmok
{
    class Window;
    class Camera;

    struct DARMOK_EXPORT FreelookConfig final
    {
        float lookSensitivity = 50.F;
        float moveSensitivity = 10.F;
        glm::vec2 maxLookAngle = glm::vec2(30, 30);
        InputEvents enableEvents = { KeyboardInputEvent{ KeyboardKey::F7 } };

        InputDirs moveLeft = {
            KeyboardInputEvent{ KeyboardKey::Left },
            KeyboardInputEvent{ KeyboardKey::KeyA },
            GamepadInputDir{ GamepadStick::Left, InputDirType::Left }
        };
        InputDirs moveRight = {
            KeyboardInputEvent{ KeyboardKey::Right },
            KeyboardInputEvent{ KeyboardKey::KeyD },
            GamepadInputDir{ GamepadStick::Left, InputDirType::Right }
        };
        InputDirs moveForward = {
            KeyboardInputEvent{ KeyboardKey::Up },
            KeyboardInputEvent{ KeyboardKey::KeyW },
            GamepadInputDir{ GamepadStick::Left, InputDirType::Up }
        };
        InputDirs moveBackward = {
            KeyboardInputEvent{ KeyboardKey::Down },
            KeyboardInputEvent{ KeyboardKey::KeyS },
            GamepadInputDir{ GamepadStick::Left, InputDirType::Down }
        };
        InputDirs lookLeft = {
            MouseInputDir{ MouseAnalog::Position, InputDirType::Left },
            GamepadInputDir{ GamepadStick::Right, InputDirType::Left }
        };
        InputDirs lookRight = {
            MouseInputDir{ MouseAnalog::Position, InputDirType::Right },
            GamepadInputDir{ GamepadStick::Right, InputDirType::Right }
        };
        InputDirs lookUp = {
            MouseInputDir{ MouseAnalog::Position, InputDirType::Up },
            GamepadInputDir{ GamepadStick::Right, InputDirType::Up }
        };
        InputDirs lookDown = {
            MouseInputDir{ MouseAnalog::Position, InputDirType::Down },
            GamepadInputDir{ GamepadStick::Right, InputDirType::Down }
        };
    };

    class DARMOK_EXPORT BX_NO_VTABLE IFreelookListener
    {
    public:
        virtual ~IFreelookListener() = default;
        virtual void onFreelookEnable(bool enabled) = 0;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IFreelookListenerFilter
    {
    public:
        virtual ~IFreelookListenerFilter() = default;
        virtual bool operator()(const IFreelookListener& listener, entt::id_type type) const = 0;
    };

    class DARMOK_EXPORT FreelookController final : public ISceneComponent, public IInputEventListener
    {
    public:
        using Config = FreelookConfig;
        FreelookController(Camera& cam, const Config& config = {}) noexcept;
        void init(Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void update(float deltaTime) noexcept override;
        void onInputEvent(const std::string& tag) noexcept override;
        FreelookController& setEnabled(bool enabled) noexcept;
        bool isEnabled() const noexcept;
        FreelookController& addListener(IFreelookListener& listener) noexcept;
        FreelookController& addListener(std::unique_ptr<IFreelookListener>&& listener) noexcept;
        bool removeListener(const IFreelookListener& listener) noexcept;
        size_t removeListeners(const IFreelookListenerFilter& filter) noexcept;
    private:
        OptionalRef<Scene> _scene;
        OptionalRef<Input> _input;
        OptionalRef<Window> _win;
        Camera& _cam;
        WindowCursorMode _winCursorMode;
        glm::quat _rot;
        glm::vec3 _pos;
        glm::vec3 _scale;
        bool _enabled;
        Config _config;
        OwnRefCollection<IFreelookListener> _listeners;
    };
}