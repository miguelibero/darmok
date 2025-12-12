#ifdef _DEBUG

#include "detail/rmlui_debug.hpp"
#include <RmlUi/Debugger.h>
#include <darmok/rmlui.hpp>
#include <darmok/window.hpp>
#include <darmok/scene.hpp>
#include "detail/rmlui.hpp"

namespace darmok
{
	RmluiDebuggerComponentImpl::RmluiDebuggerComponentImpl(const Config& config) noexcept
		: _config(config)
        , _originalCursorMode(WindowCursorMode::Normal)
	{
	}

    RmluiDebuggerComponentImpl::~RmluiDebuggerComponentImpl() noexcept = default;

    const std::string RmluiDebuggerComponentImpl::_tag = "debugger";

    expected<void, std::string> RmluiDebuggerComponentImpl::init(App& app) noexcept
	{
        _app = app;
        _originalCursorMode = app.getWindow().getCursorMode();
        app.getInput().addListener(_tag, _config.enableEvents, *this);
        return {};
	}

    expected<void, std::string> RmluiDebuggerComponentImpl::shutdown() noexcept
    {
        if (!_app)
        {
            return unexpected<std::string>{"uninitialized" };
        }
        if (_canvas)
        {
            Rml::Debugger::Shutdown();
            _app->getWindow().requestCursorMode(_originalCursorMode);
        }
        _app->getInput().removeListener(_tag, *this);
        _app.reset();
        return {};
    }

    expected<void, std::string> RmluiDebuggerComponentImpl::onInputEvent(const std::string& tag) noexcept
    {
        toggle();
        return {};
    }

    bool RmluiDebuggerComponentImpl::isEnabled() const noexcept
    {
        return !_canvas.empty();
    }

    void RmluiDebuggerComponentImpl::updateCanvases()
    {
        _canvases.clear();
        auto comp = _app->getComponent<SceneAppComponent>();
        if (!comp)
        {
            return;
        }

        for (auto scene : comp->getScenes())
        {
            auto view = scene->getComponents<RmluiCanvas>();
            for (auto [entity, canvas] : view.each())
            {
                _canvases.emplace_back(canvas);
            }
        }
    }

    void RmluiDebuggerComponentImpl::toggle() noexcept
    {
        if (_canvas)
        {
            Rml::Debugger::Shutdown();
        }
        updateCanvases();

        if (!_canvas)
        {
            if (!_canvases.empty())
            {
                _canvas = _canvases.front().get();
            }
        }
        else
        {
            auto ptr = &_canvas.value();
            auto itr = std::find_if(_canvases.begin(), _canvases.end(), [ptr](auto& canvas) { return &canvas.get() == ptr; });
            if (itr != _canvases.end())
            {
                ++itr;
            }
            if (itr == _canvases.end())
            {
                _canvas.reset();
            }
            else
            {
                _canvas = itr->get();
            }
        }

        bool active = !_canvas.empty();
        if (active)
        {
            auto ctxt = &_canvas->getContext();
            Rml::Debugger::Initialise(ctxt);
            Rml::Debugger::SetContext(ctxt);
            Rml::Debugger::SetVisible(true);
        }
        else
        {
            Rml::Debugger::Shutdown();
        }

        if (_app)
        {
            auto& win = _app->getWindow();
            win.requestCursorMode(active ? WindowCursorMode::Normal : _originalCursorMode);
        }
    }

    RmluiDebuggerComponent::RmluiDebuggerComponent(const Config& config) noexcept
        : _impl{ std::make_unique<RmluiDebuggerComponentImpl>(config) }
    {
    }

    RmluiDebuggerComponent::~RmluiDebuggerComponent() noexcept = default;

    void RmluiDebuggerComponent::toggle() noexcept
    {
        return _impl->toggle();
    }

    bool RmluiDebuggerComponent::isEnabled() const noexcept
    {
        return _impl->isEnabled();
    }

    expected<void, std::string> RmluiDebuggerComponent::init(App& app) noexcept
    {
        return _impl->init(app);
    }

    expected<void, std::string> RmluiDebuggerComponent::shutdown() noexcept
    {
        return _impl->shutdown();
    }
}

#endif