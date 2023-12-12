#pragma once

#include <darmok/app.hpp>
#include <memory>

namespace darmok
{
	class ImguiViewComponentImpl;

	class BX_NO_VTABLE IImguiRenderer
	{
	public:
		virtual ~IImguiRenderer() = default;
		virtual void imguiRender(bgfx::ViewId viewId) = 0;
	};

    class ImguiViewComponent final : public ViewComponent
    {
    public:
		ImguiViewComponent(IImguiRenderer& updater, float fontSize = 18.0f);

		void init(bgfx::ViewId viewId) override;
		void shutdown() override;
		void render() override;
		void updateLogic(float dt) override;
	private:
		std::unique_ptr<ImguiViewComponentImpl> _impl;
    };
}