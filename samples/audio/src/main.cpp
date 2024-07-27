

#include <darmok/app.hpp>
#include <darmok/imgui.hpp>
#include <bgfx/bgfx.h>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace
{
	using namespace darmok;

	class AudioSampleApp : public App, public IImguiRenderer
	{
	public:
		void init() override
		{
			App::init();

			// Enable debug text.
			setDebugFlag(BGFX_DEBUG_TEXT);
			
			auto& imgui = addComponent<darmok::ImguiAppComponent>(*this);
			// the current imgui context is static, we have to do this if darmok is dynamically linked
			// to set the static in this part
			ImGui::SetCurrentContext(imgui.getContext());
		}

		void imguiRender()
		{
			if (ImGui::Button("Play Sound"))
			{
			}
		}

		void render() const override
		{
			App::render();

			bgfx::dbgTextPrintf(0, 6, 0x0f, "Audio");
		}
	};

}

DARMOK_RUN_APP(AudioSampleApp);
