#ifdef _DEBUG

#include "rmlui_debug.hpp"
#include <RmlUi/Debugger.h>
#include <darmok/rmlui.hpp>
#include <darmok/window.hpp>
#include "rmlui.hpp"

namespace darmok
{
	RmluiDebuggerComponentImpl::RmluiDebuggerComponentImpl(const Config& config) noexcept
		: _config(config)
	{
	}

	RmluiDebuggerComponentImpl::~RmluiDebuggerComponentImpl() noexcept
	{
		shutdown();
	}

    const std::string RmluiDebuggerComponentImpl::_tag = "debugger";

	void RmluiDebuggerComponentImpl::init(Scene& scene, App& app) noexcept
	{
        _input = app.getInput();
        _win = app.getWindow();
        _originalCursorMode = _win->getCursorMode();
        if (_config.enableEvent)
        {
            _input->addListener(_tag, _config.enableEvent.value(), *this);
        }
	}

    void RmluiDebuggerComponentImpl::shutdown() noexcept
    {
        if (!_input)
        {
            return;
        }
        if (_canvas)
        {
            Rml::Debugger::Shutdown();
            _win->requestCursorMode(_originalCursorMode);
        }
        _input->removeListener(_tag, *this);
        _input.reset();
        _win.reset();
    }

    void RmluiDebuggerComponentImpl::onInputEvent(const std::string& tag) noexcept
    {
        toggle();
    }

    bool RmluiDebuggerComponentImpl::isEnabled() const noexcept
    {
        return !_canvas.empty();
    }

    void RmluiDebuggerComponentImpl::toggle() noexcept
    {
        if (_canvas)
        {
            Rml::Debugger::Shutdown();
        }
        std::vector<std::reference_wrapper<RmluiCanvas>> canvases;
        for (auto entity : _scene->getComponentView<RmluiCanvas>())
        {
            canvases.emplace_back(_scene->getComponent<RmluiCanvas>(entity).value());
        }

        if (!_canvas)
        {
            if (!canvases.empty())
            {
                _canvas = canvases.front().get();
            }
        }
        else
        {
            auto ptr = &_canvas.value();
            auto itr = std::find_if(canvases.begin(), canvases.end(), [ptr](auto& canvas) { return &canvas.get() == ptr; });
            if (itr != canvases.end())
            {
                ++itr;
            }
            if (itr == canvases.end())
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
            _win->requestCursorMode(WindowCursorMode::Normal);
        }
        else
        {
            Rml::Debugger::Shutdown();
            _win->requestCursorMode(_originalCursorMode);
        }

    }

    RmluiDebuggerComponent::RmluiDebuggerComponent(const Config& config) noexcept
        : _impl(std::make_unique<RmluiDebuggerComponentImpl>(config))
    {
    }

    RmluiDebuggerComponent::~RmluiDebuggerComponent() noexcept
    {
        // empty on purpose
    }

    void RmluiDebuggerComponent::toggle() noexcept
    {
        return _impl->toggle();
    }

    bool RmluiDebuggerComponent::isEnabled() const noexcept
    {
        return _impl->isEnabled();
    }

    void RmluiDebuggerComponent::init(Scene& scene, App& app)
    {
        return _impl->init(scene, app);
    }

    void RmluiDebuggerComponent::shutdown() noexcept
    {
        _impl->shutdown();
    }
}

#endif