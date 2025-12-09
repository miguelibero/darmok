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
	class MiniaudioDecoder final
	{
	public:
		~MiniaudioDecoder() noexcept;
		MiniaudioDecoder(const MiniaudioDecoder& other) = delete;
		MiniaudioDecoder& operator=(const MiniaudioDecoder& other) = delete;
		MiniaudioDecoder(MiniaudioDecoder&& other) = default;
		MiniaudioDecoder& operator=(MiniaudioDecoder&& other) noexcept;

		static expected<MiniaudioDecoder, std::string> create(DataView data) noexcept;

		expected<float, std::string> getDuration() const noexcept;
		operator ma_decoder* () noexcept;
	private:
		MiniaudioDecoder(std::unique_ptr<ma_decoder> decoder) noexcept;
		bool uninit() noexcept;
		std::unique_ptr<ma_decoder> _decoder;
	};

	class SoundImpl final
	{
	public:
		static expected<SoundImpl, std::string> create(Data data) noexcept;
		DataView getData() noexcept;
		float getDuration() const noexcept;
	private:
		Data _data;
		MiniaudioDecoder _decoder;
		float _duration;
		SoundImpl(Data data, MiniaudioDecoder decoder, float duration) noexcept;
	};

	class MusicImpl final
	{
	public:
		static expected<MusicImpl, std::string> create(Data data) noexcept;
		DataView getData() noexcept;
		float getDuration() const noexcept;
	private:
		Data _data;
		MiniaudioDecoder _decoder;
		float _duration;
		MusicImpl(Data data, MiniaudioDecoder decoder, float duration) noexcept;
	};

	class MiniaudioSoundGroup final
	{
	public:
		static expected<MiniaudioSoundGroup, std::string> create(ma_engine& engine, ma_uint32 flags = 0) noexcept;
		~MiniaudioSoundGroup() noexcept;
		MiniaudioSoundGroup(const MiniaudioSoundGroup& other) = delete;
		MiniaudioSoundGroup& operator=(const MiniaudioSoundGroup& other) = delete;
		MiniaudioSoundGroup(MiniaudioSoundGroup&& other) = default;
		MiniaudioSoundGroup& operator=(MiniaudioSoundGroup&& other) noexcept;

		operator ma_sound_group*() noexcept;
		float getVolume() const noexcept;
		void setVolume(float val) noexcept;
	private:
		MiniaudioSoundGroup(std::unique_ptr<ma_sound_group> group) noexcept;
		bool uninit() noexcept;
		std::unique_ptr<ma_sound_group> _group;
	};

	class MiniaudioSound final
	{
	public:
		static expected<MiniaudioSound, std::string> create(DataView data, ma_engine& engine, const OptionalRef<MiniaudioSoundGroup>& group = nullptr) noexcept;
		~MiniaudioSound() noexcept;
		MiniaudioSound(const MiniaudioSound& other) = delete;
		MiniaudioSound& operator=(const MiniaudioSound& other) = delete;
		MiniaudioSound(MiniaudioSound&& other) = default;
		MiniaudioSound& operator=(MiniaudioSound&& other) noexcept;

		expected<void, std::string> start() noexcept;
		expected<void, std::string> stop() noexcept;
		bool atEnd() const noexcept;
		bool isPlaying() const noexcept;
		void setPosition(const glm::vec3& pos) noexcept;
		void setLooping(bool val) noexcept;
	private:
		std::unique_ptr<ma_sound> _sound;
		MiniaudioDecoder _decoder;

		MiniaudioSound(std::unique_ptr<ma_sound> sound, MiniaudioDecoder decoder) noexcept;
		bool uninit() noexcept;
	};

	class AudioSystemImpl final
	{
	public:
		AudioSystemImpl() = default;
		~AudioSystemImpl() noexcept;
		AudioSystemImpl(const AudioSystemImpl& other) = delete;
		AudioSystemImpl& operator=(const AudioSystemImpl& other) = delete;

		expected<void, std::string> init() noexcept;
		void shutdown() noexcept;
		void update() noexcept;
		expected<void, std::string> play(const std::shared_ptr<Sound>& sound) noexcept;
		expected<void, std::string> play(const std::shared_ptr<Sound>& sound, const glm::vec3& pos) noexcept;
		expected<void, std::string> play(const std::shared_ptr<Music>& music) noexcept;

		[[nodiscard]] float getVolume(AudioGroup group) const noexcept;
		void setVolume(AudioGroup group, float v) noexcept;
		void stopMusic() noexcept;
		expected<void, std::string> pauseMusic() noexcept;
		[[nodiscard]] MusicState getMusicState() const noexcept;
	private:
		std::unique_ptr<ma_engine> _engine;
		std::optional<MiniaudioSoundGroup> _soundGroup;
		std::optional<MiniaudioSoundGroup> _musicGroup;

		expected<std::reference_wrapper<MiniaudioSound>, std::string> createMiniaudioSound(const std::shared_ptr<Sound>& sound) noexcept;

		struct MusicElement
		{
			MiniaudioSound miniaudio;
			std::shared_ptr<Music> darmok;
		};

		std::optional<MusicElement> _music;

		struct SoundElement
		{
			MiniaudioSound miniaudio;
			std::shared_ptr<Sound> darmok;
		};

		std::vector<SoundElement> _sounds;
	};
}