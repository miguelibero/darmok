#include <darmok/freelook.hpp>
#include <darmok/app.hpp>
#include <darmok/window.hpp>
#include <darmok/camera.hpp>
#include <darmok/math.hpp>
#include <darmok/shape.hpp>
#include <darmok/transform.hpp>
#include <darmok/glm_serialize.hpp>
#include <darmok/scene_serialize.hpp>

namespace darmok
{
    FreelookController::FreelookController(OptionalRef<Camera> cam, const Definition& def) noexcept
        : _def{ def }
        , _enabled{ false }
        , _winCursorMode{ WindowCursorMode::Normal }
        , _cam{ cam }
        , _pos{ 0 }
        , _rot{ 0, 0, 0, 1 }
        , _scale{ 1 }
    {
    }

    FreelookController& FreelookController::addListener(IFreelookListener& listener) noexcept
    {
        _listeners.insert(listener);
        return *this;
    }

    FreelookController& FreelookController::addListener(std::unique_ptr<IFreelookListener> listener) noexcept
    {
        _listeners.insert(std::move(listener));
        return *this;
    }

    bool FreelookController::removeListener(const IFreelookListener& listener) noexcept
    {
        return _listeners.erase(listener);
    }

    size_t FreelookController::removeListeners(const IFreelookListenerFilter& filter) noexcept
    {
        return _listeners.eraseIf(filter);
    }

    expected<void, std::string> FreelookController::init(Scene& scene, App& app) noexcept
    {
        _scene = scene;
        _input = app.getInput();
        _win = app.getWindow();        
        return doInit();
    };

    expected<void, std::string> FreelookController::doInit() noexcept
    {
        if (!_cam && _scene)
        {
            for (auto entity : _scene->getComponents<Camera>())
            {
                if (_cam = _scene->getComponent<Camera>(entity))
                {
                    break;
                }
            }
        }
        _input->addListener("freelook", _def.enable_events(), *this);
        return {};
    }

    expected<void, std::string> FreelookController::load(const Definition& def, IComponentLoadContext& context) noexcept
    {
        if (_input)
        {
            _input->removeListener(*this);
        }
        _def = def;
        if (_def.has_camera())
        {
            auto entity = context.getEntity(_def.camera());
            _cam = context.getScene().getComponent<Camera>(entity);
        }
        return doInit();
    }

    expected<void, std::string> FreelookController::shutdown() noexcept
    {
        if (_input)
        {
            _input->removeListener(*this);
        }
        _input.reset();
        _win.reset();
        return {};
    }

    expected<void, std::string> FreelookController::onInputEvent(const std::string& tag) noexcept
    {
        return setEnabled(!_enabled);
    }

    expected<void, std::string> FreelookController::setEnabled(bool enabled) noexcept
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

        if (enabled && _cam)
        {
            if (auto trans = _cam->getTransform())
            {
                Math::decompose(trans->getLocalMatrix(), _pos, _rot, _scale);
            }
        }
        std::vector<std::string> errors;
        for (auto& listener : _listeners)
        {
            auto result = listener.onFreelookEnable(enabled);
            if (!result)
            {
                errors.push_back(std::move(result).error());
            }
        }

        return StringUtils::joinExpectedErrors(errors);
    }

    bool FreelookController::isEnabled() const noexcept
    {
        return _enabled;
    }

    expected<void, std::string> FreelookController::update(float deltaTime) noexcept
    {
        if (!_enabled || !_cam)
        {
            return {};
        }
        auto& lookDirs = _def.look();
        glm::vec2 lookRot{
            _input->getAxis(lookDirs.left(), lookDirs.right()),
            _input->getAxis(lookDirs.down(), lookDirs.up())
        };
        lookRot *= _def.look_sensitivity() * deltaTime;
        auto lookAngle = convert<glm::vec2>(_def.max_look_angle());
        lookRot = glm::clamp(lookRot, -lookAngle, lookAngle);
        lookRot = glm::radians(lookRot);
        
        _rot = glm::quat{ glm::vec3{ 0, lookRot.x, 0 } } * _rot * glm::quat{ glm::vec3{-lookRot.y, 0, 0} };

        auto& moveDirs = _def.move();
        glm::vec3 dir(
            _input->getAxis(moveDirs.left(), moveDirs.right()),
            0,
            _input->getAxis(moveDirs.backward(), moveDirs.forward())
        );

        dir = _rot * dir;
        _pos += dir * _def.move_sensitivity() * deltaTime;

        auto mtx = Math::transform(_pos, _rot, _scale);

        if (auto trans = _cam->getTransform())
        {
            trans->setLocalMatrix(mtx);
        }
        return {};
    }


    FreelookController::Definition FreelookController::createDefinition() noexcept
    {
        Definition def;
		def.set_look_sensitivity(50.F);
        def.set_move_sensitivity(10.F);
		def.mutable_max_look_angle()->set_x(30.F);
        def.mutable_max_look_angle()->set_y(30.F);

		def.mutable_enable_events()->Add(Keyboard::createInputEvent(Keyboard::Definition::KeyF7));
        *def.mutable_move() = Input::createMoveDirsDefinition();
        *def.mutable_look() = Input::createLookDirsDefinition();
        return def;
    }
}