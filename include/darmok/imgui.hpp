#pragma once

#include <darmok/app.hpp>
#include <memory>

namespace darmok
{
	class ImguiViewComponentImpl;

	class BX_NO_VTABLE ImguiUpdaterI
	{
	public:
		virtual ~ImguiUpdaterI() = default;
		virtual void imguiUpdate(const InputState& input, bgfx::ViewId viewId, const WindowHandle& window) = 0;
	};

    class ImguiViewComponent final : public ViewComponent
    {
    public:
		ImguiViewComponent(ImguiUpdaterI& updater, float fontSize = 18.0f);
		void init(bgfx::ViewId viewId);
		void shutdown();

		void update(const InputState& input, const WindowHandle& window) override;
	private:
		std::unique_ptr<ImguiViewComponentImpl> _impl;
    };
}