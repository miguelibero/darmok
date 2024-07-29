#pragma once
#include <darmok/audio_mini.hpp>
#include <darmok/data.hpp>
#include <darmok/optional_ref.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <optional>
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
	public:
		MusicImpl(Data&& data);
		DataView getData() noexcept;
	private:
		Data _data;
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

	class MiniaudioSoundGroup final
	{
	public:
		MiniaudioSoundGroup(ma_engine& engine, ma_uint32 flags = 0) noexcept;
		~MiniaudioSoundGroup() noexcept;
		operator ma_sound_group*() noexcept;
		float getVolume() const noexcept;
		void setVolume(float v) noexcept;
	private:
		ma_sound_group _group;
	};

	class MiniaudioSound final
	{
	public:
		MiniaudioSound(DataView data, ma_engine& engine, const OptionalRef<MiniaudioSoundGroup>& group = nullptr) noexcept;
		~MiniaudioSound() noexcept;
		void start();
		void stop();
		bool atEnd() const noexcept;
		bool isPlaying() const noexcept;
		void setPosition(const glm::vec3& pos) noexcept;
		void setLooping(bool v) noexcept;
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

		float getVolume(AudioGroup group) const;
		void setVolume(AudioGroup group, float v);
		void stopMusic();
		void pauseMusic();
		std::shared_ptr<Music> getRunningMusic() noexcept;
		ma_engine& getEngine();
	private:
		ma_engine _engine;
		std::unique_ptr<MiniaudioSoundGroup> _soundGroup;
		std::unique_ptr<MiniaudioSoundGroup> _musicGroup;

		MiniaudioSound& createMiniaudioSound(const std::shared_ptr<Sound>& sound);

		struct MusicElement
		{
			std::unique_ptr<MiniaudioSound> miniaudio;
			std::shared_ptr<Music> darmok;
		};

		std::optional<MusicElement> _music;

		struct SoundElement
		{
			std::unique_ptr<MiniaudioSound> miniaudio;
			std::shared_ptr<Sound> darmok;
		};

		std::vector<SoundElement> _sounds;
	};
}