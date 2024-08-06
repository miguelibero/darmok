#ifdef _DEBUG

#include "rmlui_debug.hpp"
#include <RmlUi/Debugger.h>
#include <darmok/rmlui.hpp>
#include <darmok/window.hpp>
#include "rmlui.hpp"

namespace darmok
{
	RmluiDebuggerAppComponentImpl::RmluiDebuggerAppComponentImpl(RmluiAppComponent& comp, const Config& config) noexcept
		: _comp(comp)
		, _config(config)
	{
	}

	RmluiDebuggerAppComponentImpl::~RmluiDebuggerAppComponentImpl() noexcept
	{
		shutdown();
	}

    const std::string RmluiDebuggerAppComponentImpl::_tag = "debugger";

	void RmluiDebuggerAppComponentImpl::init(App& app) noexcept
	{
        _input = app.getInput();
        _win = app.getWindow();
        _originalCursorMode = _win->getCursorMode();
        if (_config.enableEvent)
        {
            _input->addListener(_tag, _config.enableEvent.value(), *this);
        }
	}

    void RmluiDebuggerAppComponentImpl::shutdown() noexcept
    {
        if (!_input)
        {
            return;
        }
        if (_view)
        {
            Rml::Debugger::Shutdown();
            _win->requestCursorMode(_originalCursorMode);
        }
        _input->removeListener(_tag, *this);
        _input.reset();
        _win.reset();
    }

    void RmluiDebuggerAppComponentImpl::onInputEvent(const std::string& tag) noexcept
    {
        toggle();
    }

    bool RmluiDebuggerAppComponentImpl::isEnabled() const noexcept
    {
        return !_view.empty();
    }

    void RmluiDebuggerAppComponentImpl::toggle() noexcept
    {
        if (_view)
        {
            Rml::Debugger::Shutdown();
        }
        auto& views = _comp.getImpl().getViews();
        if (!_view)
        {
            if (!views.empty())
            {
                _view = *views.front();
            }
        }
        else
        {
            auto ptr = &_view.value();
            auto itr = std::find_if(views.begin(), views.end(), [ptr](auto& view) { return view.get() == ptr; });
            if (itr != views.end())
            {
                ++itr;
            }
            if (itr == views.end())
            {
                _view.reset();
            }
            else
            {
                _view = **itr;
            }
        }

        bool active = !_view.empty();
        if (active)
        {
            auto ctxt = &_view->getContext();
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

    RmluiDebuggerAppComponent::RmluiDebuggerAppComponent(RmluiAppComponent& comp, const Config& config) noexcept
        : _impl(std::make_unique<RmluiDebuggerAppComponentImpl>(comp, config))
    {
    }

    RmluiDebuggerAppComponent::~RmluiDebuggerAppComponent() noexcept
    {
        // empty on purpose
    }

    void RmluiDebuggerAppComponent::toggle() noexcept
    {
        return _impl->toggle();
    }

    bool RmluiDebuggerAppComponent::isEnabled() const noexcept
    {
        return _impl->isEnabled();
    }

    void RmluiDebuggerAppComponent::init(App& app)
    {
        return _impl->init(app);
    }

    void RmluiDebuggerAppComponent::shutdown() noexcept
    {
        _impl->shutdown();
    }
}

#endif