#pragma once
#include <darmok/audio_mini.hpp>
#include <darmok/data.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <miniaudio.h>

namespace bx
{
	struct FileReaderI;
	struct FileWriterI;
}

namespace darmok
{
	class SoundImpl final
	{
	public:
		SoundImpl(Data&& data);
		~SoundImpl() noexcept;
		ma_decoder& getDataSource() noexcept;
	private:
		ma_decoder _decoder;
		Data _data;
	};

	class MusicImpl final
	{
	};

	class AudioPlayerImpl final
	{
	public:
		void init();
		void shutdown();
		void play(const std::shared_ptr<Sound>& sound);
		void play(const std::shared_ptr<Sound>& sound, const glm::vec3& pos);
		void play(const std::shared_ptr<Music>& music);
		bool stopMusic();
		bool pauseMusic();
		std::shared_ptr<Music> getRunningMusic() noexcept;
	private:
		ma_engine _engine;
		std::vector<ma_sound> _playingSounds;
	};
}