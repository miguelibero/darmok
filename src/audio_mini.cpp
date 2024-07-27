#include <darmok/audio.hpp>
#include "audio_mini.hpp"
#include <stdexcept>
#include <bx/bx.h>
#include <fstream>


#ifdef __APPLE__
#define MA_NO_RUNTIME_LINKING
#endif
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace darmok
{
    struct AudioUtils final
    {
        static void checkResult(ma_result result)
        {
            if (result != MA_SUCCESS)
            {
                throw std::runtime_error(ma_result_description(result));
            }
        }
    };

    SoundImpl::SoundImpl(Data&& data)
        : _data(std::move(data))
    {
        auto result = ma_decoder_init_memory(_data.ptr(), _data.size(), nullptr, &_decoder);
        AudioUtils::checkResult(result);
    }

    SoundImpl::~SoundImpl() noexcept
    {
        ma_decoder_uninit(&_decoder);
    }

    ma_decoder& SoundImpl::getDataSource() noexcept
    {
        return _decoder;
    }

    Sound::Sound(std::unique_ptr<SoundImpl>&& impl)
        : _impl(std::move(impl))
    {
    }

    Sound::~Sound()
    {
        // empty on purpose
    }

    SoundImpl& Sound::getImpl() noexcept
    {
        return *_impl;
    }

    const SoundImpl& Sound::getImpl() const noexcept
    {
        return *_impl;
    }

    Music::Music(std::unique_ptr<MusicImpl>&& impl)
    {
    }

    Music::~Music()
    {
        // empty on purpose
    }

    MusicImpl& Music::getImpl() noexcept
    {
        return *_impl;
    }

    const MusicImpl& Music::getImpl() const noexcept
    {
        return *_impl;
    }

    MiniaudioSoundLoader::MiniaudioSoundLoader(IDataLoader& dataLoader)
        : _dataLoader(dataLoader)
    {
    }

    std::shared_ptr<Sound> MiniaudioSoundLoader::operator()(std::string_view name)
    {
        auto data = _dataLoader(name);
        return std::make_shared<Sound>(std::make_unique<SoundImpl>(std::move(data)));
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
        ma_engine_config config = ma_engine_config_init();
        auto result = ma_engine_init(&config, &_engine);
        AudioUtils::checkResult(result);
    }

    void AudioPlayerImpl::shutdown()
    {
        ma_engine_uninit(&_engine);
    }

    void AudioPlayerImpl::play(const std::shared_ptr<Sound>& sound)
    {
        auto& source = sound->getImpl().getDataSource();
        auto& maSound = _playingSounds.emplace_back();
        auto result = ma_sound_init_from_data_source(&_engine, &source, 0, nullptr, &maSound);
        AudioUtils::checkResult(result);
        result = ma_sound_start(&maSound);
        AudioUtils::checkResult(result);
    }

    void AudioPlayerImpl::play(const std::shared_ptr<Sound>& sound, const glm::vec3& pos)
    {
    }

    void AudioPlayerImpl::play(const std::shared_ptr<Music>& music)
    {
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

    void AudioPlayer::play(const std::shared_ptr<Sound>& sound) noexcept
    {
        return _impl->play(sound);
    }

    void AudioPlayer::play(const std::shared_ptr<Sound>& sound, const glm::vec3& pos) noexcept
    {
        return _impl->play(sound, pos);
    }

    void AudioPlayer::play(const std::shared_ptr<Music>& music) noexcept
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