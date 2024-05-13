#pragma once

#include <darmok/app.hpp>
#include <memory>
#include <bx/bx.h>
#include <imgui.h>

namespace darmok
{
	class BX_NO_VTABLE IImguiRenderer
	{
	public:
		DLLEXPORT virtual ~IImguiRenderer() = default;
		DLLEXPORT virtual void imguiRender() = 0;
	};


	class ImguiAppComponentImpl;

    class ImguiAppComponent final : public AppComponent
    {
    public:
		DLLEXPORT ImguiAppComponent(IImguiRenderer& renderer, float fontSize = 18.0f) noexcept;

		void init(App& app) override;
		void shutdown() override;
		bgfx::ViewId render(bgfx::ViewId viewId) const override;
		void updateLogic(float dt) override;
	private:
		std::unique_ptr<ImguiAppComponentImpl> _impl;
    };
}