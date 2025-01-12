

#include <darmok/app.hpp>
#include <darmok/imgui.hpp>
#include <darmok/audio.hpp>
#include <darmok/asset.hpp>
#include <bgfx/bgfx.h>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace
{
	using namespace darmok;

	class AudioSampleAppDelegate final : public IAppDelegate, public IImguiRenderer
	{
	public:
		AudioSampleAppDelegate(App& app)
			: _app(app)
		{
		}

		void init() override
		{
			// Enable debug text.
			_app.setDebugFlag(BGFX_DEBUG_TEXT);
			
			auto& imgui = _app.addComponent<darmok::ImguiAppComponent>(*this);
			// the current imgui context is static, we have to do this if darmok is dynamically linked
			// to set the static in this part
			ImGui::SetCurrentContext(imgui.getContext());

			_sound = _app.getAssets().getSoundLoader()("sound.wav");
			_music = _app.getAssets().getMusicLoader()("music.mp3");
			auto& audio = _app.getAudio();
			_soundVolume = audio.getVolume(AudioGroup::Sound);
			_musicVolume = audio.getVolume(AudioGroup::Music);
		}

		void shutdown() override
		{
			_sound.reset();
			_music.reset();
		}

		void imguiRender()
		{
			auto& audio = _app.getAudio();
			if (ImGui::Button("Play Sound"))
			{
				audio.play(_sound);
			}

			if (ImGui::SliderFloat("Sound Volume", &_soundVolume, 0.F, 1.F))
			{
				audio.setVolume(AudioGroup::Sound, _soundVolume);
			}

			auto musicState = audio.getMusicState();
			auto running = musicState != MusicState::Stopped;
			if (ImGui::Button(running ? "Stop Music" : "Play Music"))
			{
				if (running)
				{
					audio.stopMusic();
				}
				else
				{
					audio.play(_music);
				}
			}
			if (running)
			{
				auto playing = musicState == MusicState::Playing;
				if (ImGui::Button(playing ? "Pause Music" : "Play Music"))
				{
					audio.pauseMusic();
				}
			}

			if (ImGui::SliderFloat("Music Volume", &_musicVolume, 0.F, 1.F))
			{
				audio.setVolume(AudioGroup::Music, _musicVolume);
			}
		}

		void render() const override
		{
			bgfx::dbgTextPrintf(1, 1, 0x0f, "Sound Duration %f", _sound->getDuration());
			bgfx::dbgTextPrintf(1, 2, 0x0f, "Music Duration %f", _music->getDuration());
		}
	private:
		App& _app;
		float _soundVolume = 0.F;
		float _musicVolume = 0.F;
		std::shared_ptr<Sound> _sound;
		std::shared_ptr<Music> _music;
	};

}

DARMOK_RUN_APP(AudioSampleAppDelegate);
