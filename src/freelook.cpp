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

    FreelookController& FreelookController::addListener(std::unique_ptr<IFreelookListener>&& listener) noexcept
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
        if (!_cam)
        {
            for (auto entity : scene.getComponents<Camera>())
            {
                if(_cam = scene.getComponent<Camera>(entity))
                {
                    break;
                }
            }
        }
        _scene = scene;
        _input = app.getInput();
        _win = app.getWindow();
        _input->addListener("freelook", _def.enable_events(), *this);
        return {};
    };

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
        setEnabled(!_enabled);
		return {};
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

        if (enabled && _cam)
        {
            if (auto trans = _cam->getTransform())
            {
                Math::decompose(trans->getLocalMatrix(), _pos, _rot, _scale);
            }
        }
        for (auto& listener : _listeners)
        {
            listener.onFreelookEnable(enabled);
        }

        return *this;
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
        glm::vec2 lookRot{
            _input->getAxis(_def.look_left(), _def.look_right()),
            _input->getAxis(_def.look_down(), _def.look_up())
        };
        lookRot *= _def.look_sensitivity() * deltaTime;
        auto lookAngle = convert<glm::vec2>(_def.max_look_angle());
        lookRot = glm::clamp(lookRot, -lookAngle, lookAngle);
        lookRot = glm::radians(lookRot);
        
        _rot = glm::quat(glm::vec3(0, lookRot.x, 0)) * _rot * glm::quat(glm::vec3(-lookRot.y, 0, 0));

        glm::vec3 dir(
            _input->getAxis(_def.move_left(), _def.move_right()),
            0,
            _input->getAxis(_def.move_backward(), _def.move_forward())
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

		def.mutable_enable_events()->Add()->mutable_keyboard()->set_key(Keyboard::Definition::F7);

        auto addMove = [](auto field, KeyboardKey key1, KeyboardKey key2, InputDirType gamepadDir)
        {
            field->Add(Keyboard::createInputDir(key1));
            field->Add(Keyboard::createInputDir(key2));
            field->Add(Gamepad::createInputDir(gamepadDir));
        };
        addMove(def.mutable_move_left(), Keyboard::Definition::KeyLeft, Keyboard::Definition::KeyA, Input::Definition::LeftDir);
        addMove(def.mutable_move_right(), Keyboard::Definition::KeyRight, Keyboard::Definition::KeyD, Input::Definition::RightDir);
        addMove(def.mutable_move_forward(), Keyboard::Definition::KeyUp, Keyboard::Definition::KeyW, Input::Definition::UpDir);
        addMove(def.mutable_move_backward(), Keyboard::Definition::KeyDown, Keyboard::Definition::KeyS, Input::Definition::DownDir);

        auto addLook = [](auto field, InputDirType dir)
        {
            field->Add(Mouse::createInputDir(dir));
            field->Add(Gamepad::createInputDir(dir));
        };
		addLook(def.mutable_look_left(), Input::Definition::LeftDir);
        addLook(def.mutable_look_right(), Input::Definition::LeftDir);
        addLook(def.mutable_look_up(), Input::Definition::UpDir);
        addLook(def.mutable_look_down(), Input::Definition::DownDir);

        return def;
    }

    expected<void, std::string> FreelookController::load(const Definition& def, IComponentLoadContext& context) noexcept
    {
        auto result = shutdown();
        if (!result)
        {
            return result;
        }
        _def = def;

        if (def.has_camera())
        {
            auto entity = context.getEntity(def.camera());
            _cam = context.getScene().getComponent<Camera>(entity);
        }

        return init(context.getScene(), context.getApp());
    }
}