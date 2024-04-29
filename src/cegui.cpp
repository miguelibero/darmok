#include "cegui.hpp"
#include "cegui/renderer.hpp"
#include "cegui/resource.hpp"
#include <darmok/cegui.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include <CEGUI/System.h>

namespace darmok
{
	CeguiAppComponentImpl::CeguiAppComponentImpl() noexcept
	{
	}

	void CeguiAppComponentImpl::init(App& app)
	{
		_renderer = std::make_unique<CeguiRenderer>(app);
		_resourceProvider = std::make_unique<CeguiResourceProvider>(app.getAssets().getDataLoader());
		CEGUI::System::create(*_renderer, _resourceProvider.get());
	}

	void CeguiAppComponentImpl::shutdown()
	{
		CEGUI::System::destroy();
		_renderer.reset();
		_resourceProvider.reset();
	}

	void CeguiAppComponentImpl::updateLogic(float deltaTime)
	{
	}

	bgfx::ViewId CeguiAppComponentImpl::render(bgfx::ViewId viewId) const
	{
		return viewId;
	}

	CeguiAppComponent::CeguiAppComponent() noexcept
		: _impl(std::make_unique<CeguiAppComponentImpl>())
	{
	}

	CeguiAppComponent::~CeguiAppComponent() noexcept
	{
	}

	void CeguiAppComponent::init(App& app)
	{
		_impl->init(app);
	}

	void CeguiAppComponent::shutdown()
	{
		_impl->shutdown();
	}

	void CeguiAppComponent::updateLogic(float deltaTime)
	{
		_impl->updateLogic(deltaTime);
	}

	bgfx::ViewId CeguiAppComponent::render(bgfx::ViewId viewId) const
	{
		return _impl->render(viewId);;
	}
}