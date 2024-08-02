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

	void RmluiDebuggerAppComponentImpl::init(App& app) noexcept
	{
		Rml::Debugger::Initialise(&_comp.getView().getContext());
        _input = app.getInput();
        if (_config.enableEvent)
        {
            _input->addListener("debugger", _config.enableEvent.value(), *this);
        }
	}

    void RmluiDebuggerAppComponentImpl::shutdown() noexcept
    {
        Rml::Debugger::Shutdown();
        if (_input)
        {
            _input->removeListener("debugger", *this);
            _input.reset();
        }
    }

    void RmluiDebuggerAppComponentImpl::onInputEvent(const std::string& tag) noexcept
    {
        toggle();
    }

    void RmluiDebuggerAppComponentImpl::toggle() noexcept
    {
        auto& views = _comp.getImpl().getViews();
        if (!_view)
        {
            if (!views.empty())
            {
                _view = views.begin()->second;
            }
        }
        else
        {
            auto ptr = &_view.value();
            auto itr = std::find_if(views.begin(), views.end(), [ptr](auto& elm) { return &elm.second == ptr; });
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
                _view = itr->second;
            }
        }
        if (_view)
        {
            Rml::Debugger::SetVisible(true);
            Rml::Debugger::SetContext(&_view->getContext());
        }
        else
        {
            Rml::Debugger::SetVisible(false);
            Rml::Debugger::SetContext(nullptr);
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