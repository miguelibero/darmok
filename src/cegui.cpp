#include "cegui.hpp"
#include "cegui/renderer.hpp"
#include "cegui/resource.hpp"
#include <darmok/cegui.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include <CEGUI/System.h>
#include <CEGUI/ImageCodecModules/STB/ImageCodec.h>
#include <CEGUI/GUIContext.h>

namespace darmok
{
	CeguiAppComponentImpl::CeguiAppComponentImpl() noexcept
	{
	}

	void CeguiAppComponentImpl::init(App& app)
	{
		_renderer = std::make_unique<CeguiRenderer>(app);
		_resourceProvider = std::make_unique<CeguiResourceProvider>(app.getAssets().getDataLoader());
		CEGUI::System::create(
			*_renderer, _resourceProvider.get()
		);

		_guiContext = CEGUI::System::getSingleton().createGUIContext(_renderer->getDefaultRenderTarget());
	}

	OptionalRef<CEGUI::GUIContext> CeguiAppComponentImpl::getGuiContext() noexcept
	{
		return _guiContext;
	}

	OptionalRef<const CEGUI::GUIContext> CeguiAppComponentImpl::getGuiContext() const noexcept
	{
		return _guiContext;
	}

	void CeguiAppComponentImpl::shutdown()
	{
		CEGUI::System::destroy();
		_renderer.reset();
		_resourceProvider.reset();
		_guiContext.reset();
	}

	void CeguiAppComponentImpl::updateLogic(float deltaTime)
	{
	}

	bgfx::ViewId CeguiAppComponentImpl::render(bgfx::ViewId viewId) const
	{
		_renderer->setViewId(viewId);
		CEGUI::System::getSingleton().renderAllGUIContexts();
		return ++viewId;
	}

	void CeguiAppComponentImpl::setResourceGroupDirectory(std::string_view resourceGroup, std::string_view directory) noexcept
	{
		_resourceProvider->setResourceGroupDirectory(
			CEGUI::String(resourceGroup.data(), resourceGroup.size()),
			CEGUI::String(directory.data(), directory.size()));
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
		return _impl->render(viewId);
	}

	void CeguiAppComponent::setResourceGroupDirectory(std::string_view resourceGroup, std::string_view directory) noexcept
	{
		_impl->setResourceGroupDirectory(resourceGroup, directory);
	}

	OptionalRef<CEGUI::GUIContext> CeguiAppComponent::getGuiContext() noexcept
	{
		return _impl->getGuiContext();
	}

	OptionalRef<const CEGUI::GUIContext> CeguiAppComponent::getGuiContext() const noexcept
	{
		return _impl->getGuiContext();
	}
}