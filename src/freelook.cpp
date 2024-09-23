#include <darmok/freelook.hpp>
#include <darmok/app.hpp>
#include <darmok/window.hpp>
#include <darmok/camera.hpp>
#include <darmok/math.hpp>
#include <darmok/shape.hpp>

namespace darmok
{
    FreelookController::FreelookController(Camera& cam, const Config& config) noexcept
        : _config(config)
        , _enabled(false)
        , _winCursorMode(WindowCursorMode::Normal)
        , _cam(cam)
        , _pos(0)
        , _rot(0, 0, 0, 1)
        , _scale(1)
    {
    }

    FreelookController& FreelookController::addListener(IFreelookListener& listener) noexcept
    {
        _listeners.emplace_back(listener);
        return *this;
    }

    bool FreelookController::removeListener(IFreelookListener& listener) noexcept
    {
        auto ptr = &listener;
        auto itr = std::remove_if(_listeners.begin(), _listeners.end(), [ptr](auto& ref) { return ref == ptr; });
        if (itr == _listeners.end())
        {
            return false;
        }
        _listeners.erase(itr, _listeners.end());
        return true;

    }

    void FreelookController::init(Scene& scene, App& app) noexcept
    {
        _scene = scene;
        _input = app.getInput();
        _win = app.getWindow();
        _input->addListener("freelook", _config.enableEvents, *this);
    };

    void FreelookController::shutdown() noexcept
    {
        if (_input)
        {
            _input->removeListener(*this);
        }
        _input.reset();
        _win.reset();
    }

    void FreelookController::onInputEvent(const std::string& tag) noexcept
    {
        setEnabled(!_enabled);
    }

    FreelookController& FreelookController::setEnabled(bool enabled) noexcept
    {
        _enabled = enabled;
        if (_win)
        {
            if (enabled)
            {
                _winCursorMode = _win->getCursorMode();
                _win->requestCursorMode(WindowCursorMode::Disabled);
            }
            else
            {
                _win->requestCursorMode(_winCursorMode);
            }
        }

        if (enabled)
        {
            if (auto trans = _cam.getTransform())
            {
                Math::decompose(trans->getLocalMatrix(), _pos, _rot, _scale);
            }
        }
        for (auto listener : _listeners)
        {
            listener->onFreelookEnable(enabled);
        }

        return *this;
    }

    bool FreelookController::isEnabled() const noexcept
    {
        return _enabled;
    }

    void FreelookController::update(float deltaTime) noexcept
    {
        if (!_enabled)
        {
            return;
        }
        glm::vec2 lookRot(
            _input->getAxis(_config.lookRight, _config.lookLeft),
            _input->getAxis(_config.lookUp, _config.lookDown)
        );
        lookRot *= _config.lookSensitivity * deltaTime;
        lookRot = glm::clamp(lookRot, -_config.maxLookAngle, _config.maxLookAngle);
        lookRot = glm::radians(lookRot);
        
        _rot = glm::quat(glm::vec3(0, lookRot.x, 0)) * _rot * glm::quat(glm::vec3(-lookRot.y, 0, 0));

        glm::vec3 dir(
            _input->getAxis(_config.moveRight, _config.moveLeft),
            0,
            _input->getAxis(_config.moveForward, _config.moveBackward)
        );

        dir = _rot * dir;
        _pos += dir * _config.moveSensitivity * deltaTime;

        auto mtx = Math::transform(_pos, _rot, _scale);

        if (auto trans = _cam.getTransform())
        {
            trans->setLocalMatrix(mtx);
        }
    }
}