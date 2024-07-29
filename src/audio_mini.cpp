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
    }

    DataView SoundImpl::getData() noexcept
    {
        return _data;
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

    MusicImpl::MusicImpl(Data&& data)
        : _data(std::move(data))
    {
    }

    DataView MusicImpl::getData() noexcept
    {
        return _data;
    }

    Music::Music(std::unique_ptr<MusicImpl>&& impl)
        : _impl(std::move(impl))
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
        auto data = _dataLoader(name);
        return std::make_shared<Music>(std::make_unique<MusicImpl>(std::move(data)));
    }

    MiniaudioDecoder::MiniaudioDecoder(DataView data) noexcept
    {
        auto result = ma_decoder_init_memory(data.ptr(), data.size(), nullptr, &_decoder);
        AudioUtils::checkResult(result);
        // ma_decoder_seek_to_pcm_frame(&_decoder, 0);
    }

    MiniaudioDecoder::~MiniaudioDecoder() noexcept
    {
        ma_decoder_uninit(&_decoder);
    }

    MiniaudioDecoder::operator ma_decoder* () noexcept
    {
        return &_decoder;
    }

    MiniaudioSoundGroup::MiniaudioSoundGroup(ma_engine& engine, ma_uint32 flags) noexcept
    {
        auto result = ma_sound_group_init(&engine, flags, nullptr, &_group);
        AudioUtils::checkResult(result);
    }

    MiniaudioSoundGroup::~MiniaudioSoundGroup() noexcept
    {
        ma_sound_group_uninit(&_group);
    }

    MiniaudioSoundGroup::operator ma_sound_group*() noexcept
    {
        return &_group;
    }

    float MiniaudioSoundGroup::getVolume() const noexcept
    {
        return ma_sound_group_get_volume(&_group);
    }

    void MiniaudioSoundGroup::setVolume(float v) noexcept
    {
        ma_sound_group_set_volume(&_group, v);
    }

    MiniaudioSound::MiniaudioSound(DataView data, ma_engine& engine, const OptionalRef<MiniaudioSoundGroup>& group) noexcept
        : _decoder(data)
    {
        auto config = ma_sound_config_init();
        config.pDataSource = _decoder;
        if (group)
        {
            config.pInitialAttachment = group.value();
        }
        auto result = ma_sound_init_ex(&engine, &config, &_sound);
        AudioUtils::checkResult(result);
    }

    MiniaudioSound::~MiniaudioSound() noexcept
    {
        ma_sound_uninit(&_sound);
    }

    bool MiniaudioSound::atEnd() const noexcept
    {
        return ma_sound_at_end(&_sound);
    }

    bool MiniaudioSound::isPlaying() const noexcept
    {
        return ma_sound_is_playing(&_sound);
    }

    void MiniaudioSound::setPosition(const glm::vec3& pos) noexcept
    {
        ma_sound_set_position(&_sound, pos.x, pos.y, pos.z);
    }

    void MiniaudioSound::setLooping(bool v) noexcept
    {
        ma_sound_set_looping(&_sound, v);
    }

    void MiniaudioSound::start()
    {
        auto result = ma_sound_start(&_sound);
        AudioUtils::checkResult(result);
    }

    void MiniaudioSound::stop()
    {
        auto result = ma_sound_stop(&_sound);
        AudioUtils::checkResult(result);
    }

    void AudioPlayerImpl::init()
    {
        ma_engine_config config = ma_engine_config_init();
        auto result = ma_engine_init(&config, &_engine);
        AudioUtils::checkResult(result);

        _soundGroup = std::make_unique<MiniaudioSoundGroup>(_engine);
        _musicGroup = std::make_unique<MiniaudioSoundGroup>(_engine, MA_SOUND_FLAG_STREAM);
    }

    void AudioPlayerImpl::shutdown()
    {
        _soundGroup.reset();
        _musicGroup.reset();
        _sounds.clear();
        ma_engine_uninit(&_engine);
    }

    void AudioPlayerImpl::update()
    {
        auto it = std::remove_if(_sounds.begin(), _sounds.end(), [](auto& elm) {
            return elm.miniaudio->atEnd();
        });
        _sounds.erase(it, _sounds.end());
    }

    MiniaudioSound& AudioPlayerImpl::createMiniaudioSound(const std::shared_ptr<Sound>& sound)
    {
        auto& elm = _sounds.emplace_back(
            std::make_unique<MiniaudioSound>(
                sound->getImpl().getData(), _engine, _soundGroup.get())
        );
        return *elm.miniaudio;
    }

    void AudioPlayerImpl::play(const std::shared_ptr<Sound>& sound)
    {
        auto& maSound = createMiniaudioSound(sound);
        maSound.start();
    }

    void AudioPlayerImpl::play(const std::shared_ptr<Sound>& sound, const glm::vec3& pos)
    {
        auto& maSound = createMiniaudioSound(sound);
        maSound.setPosition(pos);
        maSound.start();
    }

    void AudioPlayerImpl::play(const std::shared_ptr<Music>& music)
    {
        _music = MusicElement{
            std::make_unique<MiniaudioSound>(
                music->getImpl().getData(), _engine, _musicGroup.get()
            ),
            music
        };
        auto& maSound = *_music->miniaudio;
        maSound.setLooping(true);
        maSound.start();
    }

    float AudioPlayerImpl::getVolume(AudioGroup group) const
    {
        switch (group)
        {
        case AudioGroup::Sound:
            if (_soundGroup)
            {
                return _soundGroup->getVolume();
            }
            break;
        case AudioGroup::Music:
            if (_musicGroup)
            {
                return _musicGroup->getVolume();
            }
            break;
        }
        return 0.F;
    }

    void AudioPlayerImpl::setVolume(AudioGroup group, float v)
    {
        switch (group)
        {
        case AudioGroup::Sound:
            if (_soundGroup)
            {
                _soundGroup->setVolume(v);
            }
            break;
        case AudioGroup::Music:
            if (_musicGroup)
            {
                _musicGroup->setVolume(v);
            }
            break;
        }
    }

    void AudioPlayerImpl::stopMusic()
    {
        _music.reset();
    }

    void AudioPlayerImpl::pauseMusic()
    {
        if (!_music || !_music->miniaudio)
        {
            return;
        }
        if(_music->miniaudio->isPlaying())
        {
            _music->miniaudio->stop();
        }
        else
        {
            _music->miniaudio->start();
        }
    }

    std::shared_ptr<Music> AudioPlayerImpl::getRunningMusic() noexcept
    {
        if (_music && _music->miniaudio && _music->miniaudio->isPlaying())
        {
            return _music->darmok;
        }
        return nullptr;
    }

    ma_engine& AudioPlayerImpl::getEngine()
    {
        return _engine;
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

    void AudioPlayer::update()
    {
        _impl->update();
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

    float AudioPlayer::getVolume(AudioGroup group) const
    {
        return _impl->getVolume(group);
    }

    void AudioPlayer::setVolume(AudioGroup group, float v)
    {
        _impl->setVolume(group, v);
    }

    void AudioPlayer::stopMusic()
    {
        return _impl->stopMusic();
    }

    void AudioPlayer::pauseMusic()
    {
        return _impl->pauseMusic();
    }

    std::shared_ptr<Music> AudioPlayer::getRunningMusic() noexcept
    {
        return _impl->getRunningMusic();
    }
}