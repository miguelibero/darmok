#ifdef _DEBUG

#include "rmlui_debug.hpp"
#include <RmlUi/Debugger.h>
#include <darmok/rmlui.hpp>
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
        auto& hostView = _comp.addViewFront(_tag);
        hostView.setInputActive(true);
        hostView.setFullscreen(true);
        hostView.setEnabled(false);
		Rml::Debugger::Initialise(&hostView.getContext());
        _input = app.getInput();
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
        _comp.removeView(_tag);
        Rml::Debugger::Shutdown();
        _input->removeListener(_tag, *this);
        _input.reset();
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
        auto& hostView = _comp.getView(_tag);
        hostView.setEnabled(active);
        hostView.setInputActive(active);
        Rml::Debugger::SetVisible(active);
        Rml::Debugger::SetContext(active ? &_view->getContext() : nullptr);
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