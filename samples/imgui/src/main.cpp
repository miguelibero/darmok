

#include <darmok/app.hpp>
#include <darmok/imgui.hpp>
#include <darmok/data.hpp>
#include <bgfx/bgfx.h>
#include <imgui.h>

namespace
{
	using namespace darmok;

	class ImguiSampleApp : public App, public IImguiRenderer
	{
	public:
		void init() override
		{
			App::init();

			_textData = "hello darmok!";

			// Enable debug text.
			setDebugFlag(BGFX_DEBUG_TEXT);
			auto& imgui = addComponent<darmok::ImguiAppComponent>(*this);
			// the current imgui context is static, we have to do this if darmok is dynamically linked
			// to set the static in this part
			ImGui::SetCurrentContext(imgui.getContext());
		}

		void imguiRender()
		{
			ImGui::TextWrapped("lala");
			ImGui::InputText("text", static_cast<char*>(_textData.ptr()), _textData.size());
		}

	protected:
		Data _textData = Data(1024);

		bgfx::ViewId render(bgfx::ViewId viewId) const override
		{
			viewId = App::render(viewId);
			
			const bgfx::Stats* stats = bgfx::getStats();

			bgfx::dbgTextPrintf(0, 1, 0x0f, "Color can be changed with ANSI \x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b[14;me\x1b[0m code too.");

			bgfx::dbgTextPrintf(80, 1, 0x0f, "\x1b[;0m    \x1b[;1m    \x1b[; 2m    \x1b[; 3m    \x1b[; 4m    \x1b[; 5m    \x1b[; 6m    \x1b[; 7m    \x1b[0m");
			bgfx::dbgTextPrintf(80, 2, 0x0f, "\x1b[;8m    \x1b[;9m    \x1b[;10m    \x1b[;11m    \x1b[;12m    \x1b[;13m    \x1b[;14m    \x1b[;15m    \x1b[0m");

			bgfx::dbgTextPrintf(0, 2, 0x0f, "Backbuffer %dW x %dH in pixels, debug text %dW x %dH in characters."
				, stats->width
				, stats->height
				, stats->textWidth
				, stats->textHeight
			);

			return viewId;
		}
	};

}

DARMOK_RUN_APP(ImguiSampleApp);
