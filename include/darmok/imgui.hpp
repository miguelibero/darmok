#pragma once

#include <darmok/app.hpp>
#include <memory>

namespace darmok
{
	class ImguiViewComponentImpl;

    class ImguiViewComponent final : public ViewComponent
    {
    public:
		ImguiViewComponent(float fontSize = 18.0f);
		void init(bgfx::ViewId viewId);
		void shutdown();

		void beforeUpdate(const InputState& input, const WindowHandle& window) override;
		void afterUpdate(const InputState& input, const WindowHandle& window) override;
	private:
		std::unique_ptr<ImguiViewComponentImpl> _impl;
    };
}