#pragma once
#include <darmok/audio_mini.hpp>
#include <darmok/data.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
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
		DataView getData() noexcept;
	private:
		Data _data;
	};

	class MusicImpl final
	{
	};

	class MiniaudioDecoder final
	{
	public:
		MiniaudioDecoder(DataView data) noexcept;
		~MiniaudioDecoder() noexcept;
		operator ma_decoder*() noexcept;
	private:
		ma_decoder _decoder;
	};

	class MiniaudioSound final
	{
	public:
		MiniaudioSound(DataView data, ma_engine& engine) noexcept;
		~MiniaudioSound() noexcept;
		void play();
		bool atEnd() const noexcept;
	private:
		ma_sound _sound;
		MiniaudioDecoder _decoder;
	};

	class AudioPlayerImpl final
	{
	public:
		void init();
		void shutdown();
		void update();
		void play(const std::shared_ptr<Sound>& sound);
		void play(const std::shared_ptr<Sound>& sound, const glm::vec3& pos);
		void play(const std::shared_ptr<Music>& music);
		bool stopMusic();
		bool pauseMusic();
		std::shared_ptr<Music> getRunningMusic() noexcept;
		ma_engine& getEngine();
	private:
		ma_engine _engine;
		std::vector<std::unique_ptr<MiniaudioSound>> _sounds;
	};
}