#include <darmok/freelook.hpp>
#include <darmok/app.hpp>
#include <darmok/window.hpp>

namespace darmok
{
    FreelookController::FreelookController(Transform& trans, const Config& config)
        : _trans(trans)
        , _config(config)
        , _enabled(false)
    {
    }

    const std::string FreelookController::_bindingsName = "freelook";

    void FreelookController::init(Scene& scene, App& app)
    {
        _input = app.getInput();
        _win = app.getWindow();
        _winCursorMode = WindowCursorMode::Normal;
        if (_config.bindingKey)
        {
            _input->addBindings(_bindingsName, {
                { _config.bindingKey.value(), true,
                    [this]() { onBindingTriggered(); }
                }
            });
        }
    };

    void FreelookController::shutdown()
    {
        if (_input && _config.bindingKey)
        {
            _input->removeBindings(_bindingsName);
        }
        _input.reset();
        _win.reset();
    }

    void FreelookController::onBindingTriggered() noexcept
    {
        setEnabled(!_enabled);
    }

    FreelookController& FreelookController::setEnabled(bool enabled) noexcept
    {
        _enabled = enabled;
        if (_win)
        {
            _win->requestCursorMode(_enabled ? WindowCursorMode::Disabled : _winCursorMode);
        }
        if (_enabled)
        {
            _initialMatrix = _trans->getLocalMatrix();
        }
        else if(_initialMatrix)
        {
            _trans->setLocalMatrix(_initialMatrix.value());
        }
        return *this;
    }

    bool FreelookController::isEnabled() const noexcept
    {
        return _enabled;
    }

    void FreelookController::update(float deltaTime)
    {
        if (!_enabled || !_input || !_trans)
        {
            return;
        }

        auto mouseRot = _input->getMouse().getPositionDelta();
        mouseRot *= _config.mouseSensitivity * deltaTime;
        mouseRot = glm::clamp(mouseRot, -_config.maxMouseAngle, _config.maxMouseAngle);
        mouseRot = glm::radians(mouseRot);
        auto rot = _trans->getRotation();
        rot = glm::quat(glm::vec3(0, mouseRot.x, 0)) * rot * glm::quat(glm::vec3(mouseRot.y, 0, 0));
        _trans->setRotation(rot);

        glm::vec3 dir(0);
        auto& kb = _input->getKeyboard();
        if (kb.getKey(KeyboardKey::Right) || kb.getKey(KeyboardKey::KeyD))
        {
            dir.x += 1;
        }
        else if (kb.getKey(KeyboardKey::Left) || kb.getKey(KeyboardKey::KeyA))
        {
            dir.x -= 1;
        }
        else if (kb.getKey(KeyboardKey::Up) || kb.getKey(KeyboardKey::KeyW))
        {
            dir.z += 1;
        }
        else if (kb.getKey(KeyboardKey::Down) || kb.getKey(KeyboardKey::KeyS))
        {
            dir.z -= 1;
        }
        dir = rot * dir;
        auto pos = _trans->getPosition();
        pos += dir * _config.keyboardSensitivity * deltaTime;
        _trans->setPosition(pos);
    }
}