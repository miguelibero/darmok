#pragma once
#include <darmok/audio_mini.hpp>
#include <miniaudio.h>
#include <glm/glm.hpp>

namespace darmok
{
	class SoundImpl final
	{
	};

	class MusicImpl final
	{

	};

	class AudioPlayerImpl final
	{
	public:
		void init();
		void shutdown();
		bool play(const std::shared_ptr<Sound>& sound) noexcept;
		bool play(const std::shared_ptr<Sound>& sound, const glm::vec3& pos) noexcept;
		bool play(const std::shared_ptr<Music>& music) noexcept;
		bool stopMusic();
		bool pauseMusic();
		std::shared_ptr<Music> getRunningMusic() noexcept;
	private:
		ma_engine _engine;
	};
}