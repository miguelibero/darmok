

#include <darmok/app.hpp>
#include <darmok/imgui.hpp>
#include <darmok/audio.hpp>
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

			auto& audio = getAudio();
			audio.loadSound("sound", "sound.wav");
			audio.loadMusic("music", "music.mp3");
			_soundVolume = audio.getVolume(AudioGroup::Sound);
			_musicVolume = audio.getVolume(AudioGroup::Music);
		}

		void imguiRender()
		{
			auto& audio = getAudio();
			if (ImGui::Button("Play Sound"))
			{
				audio.playSound("sound");
			}

			if (ImGui::SliderFloat("Sound Volume", &_soundVolume, 0.F, 1.F))
			{
				audio.setVolume(AudioGroup::Sound, _soundVolume);
			}

			auto playing = !audio.getRunningMusic().empty();
			if (ImGui::Button(playing ? "Stop Music" : "Play Music"))
			{
				if (playing)
				{
					audio.stopMusic();
				}
				else
				{
					audio.playMusic("music");
				}
			}

			if (ImGui::SliderFloat("Music Volume", &_musicVolume, 0.F, 1.F))
			{
				audio.setVolume(AudioGroup::Music, _musicVolume);
			}
		}

		void render() const override
		{
			App::render();

			bgfx::dbgTextPrintf(0, 6, 0x0f, "Audio");
		}
	private:
		float _soundVolume;
		float _musicVolume;
	};

}

DARMOK_RUN_APP(AudioSampleApp);
