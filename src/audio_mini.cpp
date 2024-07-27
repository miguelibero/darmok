#include <darmok/audio.hpp>
#include "audio_mini.hpp"

namespace darmok
{
    Sound::Sound(std::unique_ptr<SoundImpl>&& impl)
        : _impl(std::move(impl))
    {
    }

    Sound::~Sound()
    {
        // empty on purpose
    }

    Music::Music(std::unique_ptr<MusicImpl>&& impl)
    {
    }

    Music::~Music()
    {
        // empty on purpose
    }

    MiniaudioSoundLoader::MiniaudioSoundLoader(IDataLoader& dataLoader)
        : _dataLoader(dataLoader)
    {
    }

    std::shared_ptr<Sound> MiniaudioSoundLoader::operator()(std::string_view name)
    {
        return nullptr;
    }

    MiniaudioMusicLoader::MiniaudioMusicLoader(IDataLoader& dataLoader)
        : _dataLoader(dataLoader)
    {
    }

    std::shared_ptr<Music> MiniaudioMusicLoader::operator()(std::string_view name)
    {
        return nullptr;
    }

    void AudioPlayerImpl::init()
    {

    }

    void AudioPlayerImpl::shutdown()
    {

    }

    bool AudioPlayerImpl::play(const std::shared_ptr<Sound>& sound) noexcept
    {
        return false;
    }

    bool AudioPlayerImpl::play(const std::shared_ptr<Sound>& sound, const glm::vec3& pos) noexcept
    {
        return false;
    }

    bool AudioPlayerImpl::play(const std::shared_ptr<Music>& music) noexcept
    {
        return false;
    }

    bool AudioPlayerImpl::stopMusic()
    {
        return false;
    }

    bool AudioPlayerImpl::pauseMusic()
    {
        return false;
    }

    std::shared_ptr<Music> AudioPlayerImpl::getRunningMusic() noexcept
    {
        return nullptr;
    }

    AudioPlayer::AudioPlayer() noexcept
        : _impl(std::make_unique<AudioPlayerImpl>())
    {
    }

    AudioPlayer::~AudioPlayer() noexcept
    {
        // empty on purpose
    }

    void AudioPlayer::init()
    {
        _impl->init();
    }

    void AudioPlayer::shutdown()
    {
        _impl->shutdown();
    }

    bool AudioPlayer::play(const std::shared_ptr<Sound>& sound) noexcept
    {
        return _impl->play(sound);
    }

    bool AudioPlayer::play(const std::shared_ptr<Sound>& sound, const glm::vec3& pos) noexcept
    {
        return _impl->play(sound, pos);
    }

    bool AudioPlayer::play(const std::shared_ptr<Music>& music) noexcept
    {
        return _impl->play(music);
    }

    bool AudioPlayer::stopMusic()
    {
        return _impl->stopMusic();
    }

    bool AudioPlayer::pauseMusic()
    {
        return _impl->pauseMusic();
    }

    std::shared_ptr<Music> AudioPlayer::getRunningMusic() noexcept
    {
        return _impl->getRunningMusic();
    }
}