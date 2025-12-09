#include <darmok/audio.hpp>
#include "detail/audio_mini.hpp"
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
    namespace MiniaudioDetail
    {
        static expected<void, std::string> wrapExpected(ma_result result)
        {
            if (result != MA_SUCCESS)
            {
                return unexpected<std::string>{ ma_result_description(result) };
            }
            return {};
        }
    };

    SoundImpl::SoundImpl(Data data, MiniaudioDecoder decoder, float duration) noexcept
        : _data{ std::move(data) }
        , _decoder{ std::move(decoder) }
        , _duration{ duration }
    {
    }

    expected<SoundImpl, std::string> SoundImpl::create(Data data) noexcept
    {
        auto result = MiniaudioDecoder::create(data);
        if (!result)
        {
            return unexpected{ std::move(result).error() };
        }
        auto decoder = std::move(result).value();
        auto duration = decoder.getDuration();
        if (!duration)
        {
            return unexpected{ std::move(duration).error() };
        }
        return SoundImpl{ std::move(data), std::move(decoder), duration.value() };
    }

    float SoundImpl::getDuration() const noexcept
    {
        return _duration;
    }

    DataView SoundImpl::getData() noexcept
    {
        return _data;
    }

    Sound::Sound(std::unique_ptr<SoundImpl> impl) noexcept
        : _impl(std::move(impl))
    {
    }

    Sound::~Sound() = default;

    float Sound::getDuration() const noexcept
    {
        return _impl->getDuration();
    }

    SoundImpl& Sound::getImpl() noexcept
    {
        return *_impl;
    }

    const SoundImpl& Sound::getImpl() const noexcept
    {
        return *_impl;
    }

    MusicImpl::MusicImpl(Data data, MiniaudioDecoder decoder, float duration) noexcept
        : _data{ std::move(data) }
        , _decoder{ std::move(decoder) }
        , _duration{ duration }
    {
    }

    expected<MusicImpl, std::string> MusicImpl::create(Data data) noexcept
    {
        auto result = MiniaudioDecoder::create(data);
        if (!result)
        {
            return unexpected{ std::move(result).error() };
        }
        auto decoder = std::move(result).value();
        auto duration = decoder.getDuration();
        if (!duration)
        {
            return unexpected{ std::move(duration).error() };
        }
        return MusicImpl{ std::move(data), std::move(decoder), duration.value() };
    }

    float MusicImpl::getDuration() const noexcept
    {
        return _duration;
    }

    DataView MusicImpl::getData() noexcept
    {
        return _data;
    }

    Music::Music(std::unique_ptr<MusicImpl> impl) noexcept
        : _impl{ std::move(impl) }
    {
    }

    Music::~Music() = default;

    float Music::getDuration() const noexcept
    {
        return _impl->getDuration();
    }

    MusicImpl& Music::getImpl() noexcept
    {
        return *_impl;
    }

    const MusicImpl& Music::getImpl() const noexcept
    {
        return *_impl;
    }

    MiniaudioSoundLoader::MiniaudioSoundLoader(IDataLoader& dataLoader) noexcept
        : _dataLoader{ dataLoader }
    {
    }

    MiniaudioSoundLoader::Result MiniaudioSoundLoader::operator()(std::filesystem::path path) noexcept
    {
        auto dataResult = _dataLoader(path);
        if (!dataResult)
        {
            return unexpected{ std::move(dataResult).error() };
        }
        auto& data = dataResult.value();
        auto implResult = SoundImpl::create(data);
        if (!implResult)
        {
            return unexpected{ std::move(implResult).error() };
        }
        auto impl = std::make_unique<SoundImpl>(std::move(implResult).value());
        return std::make_shared<Sound>(std::move(impl));
    }

    MiniaudioMusicLoader::MiniaudioMusicLoader(IDataLoader& dataLoader) noexcept
        : _dataLoader(dataLoader)
    {
    }

    MiniaudioMusicLoader::Result MiniaudioMusicLoader::operator()(std::filesystem::path path) noexcept
    {
        auto dataResult = _dataLoader(path);
        if (!dataResult)
        {
			return unexpected<std::string>{ dataResult.error() };
        }
		auto& data = dataResult.value();
        auto implResult = MusicImpl::create(data);
        if (!implResult)
        {
            return unexpected{ std::move(implResult).error() };
        }
        auto impl = std::make_unique<MusicImpl>(std::move(implResult).value());
        return std::make_shared<Music>(std::move(impl));
    }

    expected<MiniaudioDecoder, std::string> MiniaudioDecoder::create(DataView data) noexcept
    {
        auto decoder = std::make_unique<ma_decoder>();
        auto maResult = ma_decoder_init_memory(data.ptr(), data.size(), nullptr, decoder.get());
        auto initResult = MiniaudioDetail::wrapExpected(maResult);
        if (!initResult)
        {
            return unexpected{ std::move(initResult).error() };
        }
        return MiniaudioDecoder{ std::move(decoder) };
    }

    MiniaudioDecoder::MiniaudioDecoder(std::unique_ptr<ma_decoder> decoder) noexcept
        : _decoder{ std::move(decoder) }
    {
        // ma_decoder_seek_to_pcm_frame(_decoder.get(), 0);
    }

    bool MiniaudioDecoder::uninit() noexcept
    {
        if (_decoder)
        {
            ma_decoder_uninit(_decoder.get());
            _decoder.reset();
            return true;
        }
        return false;
    }

    MiniaudioDecoder::~MiniaudioDecoder() noexcept
    {
        uninit();
    }

    MiniaudioDecoder& MiniaudioDecoder::operator=(MiniaudioDecoder&& other) noexcept
    {
        uninit();
        _decoder = std::move(other)._decoder;
        return *this;
    }

    expected<float, std::string> MiniaudioDecoder::getDuration() const noexcept
    {
        if (!_decoder)
        {
            return unexpected<std::string>{"uninitialized"};
        }
        float len = 0;
        auto maResult = ma_data_source_get_length_in_seconds(_decoder.get(), &len);
        auto result = MiniaudioDetail::wrapExpected(maResult);
        if (!result)
        {
            return unexpected{ std::move(result).error() };
        }
        return len;
    }

    MiniaudioDecoder::operator ma_decoder* () noexcept
    {
        return _decoder.get();
    }

    expected<MiniaudioSoundGroup, std::string> MiniaudioSoundGroup::create(ma_engine& engine, ma_uint32 flags) noexcept
    {
        auto group = std::make_unique<ma_sound_group>();
        auto maResult = ma_sound_group_init(&engine, flags, nullptr, group.get());
        auto initResult = MiniaudioDetail::wrapExpected(maResult);
        if (!initResult)
        {
            return unexpected{ std::move(initResult).error() };
        }
        return MiniaudioSoundGroup{ std::move(group) };
    }

    MiniaudioSoundGroup::MiniaudioSoundGroup(std::unique_ptr<ma_sound_group> group) noexcept
        : _group{ std::move(group) }
    {
    }

    MiniaudioSoundGroup& MiniaudioSoundGroup::operator=(MiniaudioSoundGroup&& other) noexcept
    {
        uninit();
        _group = std::move(other)._group;
        return *this;
    }

    bool MiniaudioSoundGroup::uninit() noexcept
    {
        if (_group)
        {
            ma_sound_group_uninit(_group.get());
            _group.reset();
            return true;
        }
        return false;
    }

    MiniaudioSoundGroup::~MiniaudioSoundGroup() noexcept
    {
        uninit();
    }

    MiniaudioSoundGroup::operator ma_sound_group*() noexcept
    {
        return _group.get();
    }

    float MiniaudioSoundGroup::getVolume() const noexcept
    {
        if (!_group)
        {
            return 0.0f;
        }
        return ma_sound_group_get_volume(_group.get());
    }

    void MiniaudioSoundGroup::setVolume(float v) noexcept
    {
        if (_group)
        {
            ma_sound_group_set_volume(_group.get(), v);
        }
    }

    expected<MiniaudioSound, std::string> MiniaudioSound::create(DataView data, ma_engine& engine, const OptionalRef<MiniaudioSoundGroup>& group) noexcept
    {
        auto decoderResult = MiniaudioDecoder::create(data);
        if (!decoderResult)
        {
            return unexpected{ std::move(decoderResult).error() };
        }
        auto decoder = std::move(decoderResult).value();
        auto config = ma_sound_config_init();
        config.pDataSource = decoder;
        if (group)
        {
            config.pInitialAttachment = group.value();
        }
        auto sound = std::make_unique<ma_sound>();
        auto maResult = ma_sound_init_ex(&engine, &config, sound.get());
        auto initResult = MiniaudioDetail::wrapExpected(maResult);
        if (!initResult)
        {
            return unexpected{ std::move(initResult).error() };
        }
        return MiniaudioSound{ std::move(sound), std::move(decoder) };
    }

    MiniaudioSound::MiniaudioSound(std::unique_ptr<ma_sound> sound, MiniaudioDecoder decoder) noexcept
        : _sound{ std::move(sound) }
        , _decoder{ std::move(decoder) }
    {
    }

    MiniaudioSound::~MiniaudioSound() noexcept
    {
        uninit();
    }

    bool MiniaudioSound::uninit() noexcept
    {
        if (_sound)
        {
            ma_sound_uninit(_sound.get());
            _sound.reset();
            return true;
        }
        return false;
    }

    MiniaudioSound& MiniaudioSound::operator=(MiniaudioSound&& other) noexcept
    {
        uninit();
        _sound = std::move(other)._sound;
        _decoder = std::move(other)._decoder;
        return *this;
    }

    bool MiniaudioSound::atEnd() const noexcept
    {
        if (!_sound)
        {
            return true;
        }
        return ma_sound_at_end(_sound.get());
    }

    bool MiniaudioSound::isPlaying() const noexcept
    {
        if (!_sound)
        {
            return false;
        }
        return ma_sound_is_playing(_sound.get());
    }

    void MiniaudioSound::setPosition(const glm::vec3& pos) noexcept
    {
        if (!_sound)
        {
            return;
        }
        ma_sound_set_position(_sound.get(), pos.x, pos.y, pos.z);
    }

    void MiniaudioSound::setLooping(bool v) noexcept
    {
        if (!_sound)
        {
            return;
        }
        ma_sound_set_looping(_sound.get(), v);
    }

    expected<void, std::string> MiniaudioSound::start() noexcept
    {
        if (!_sound)
        {
            return unexpected<std::string>{"uninitialized"};
        }
        auto maResult = ma_sound_start(_sound.get());
        return MiniaudioDetail::wrapExpected(maResult);
    }

    expected<void, std::string> MiniaudioSound::stop() noexcept
    {
        if (!_sound)
        {
            return unexpected<std::string>{"uninitialized"};
        }
        auto maResult = ma_sound_stop(_sound.get());
        return MiniaudioDetail::wrapExpected(maResult);
    }

    AudioSystemImpl::~AudioSystemImpl() noexcept
    {
        shutdown();
    }

    expected<void, std::string> AudioSystemImpl::init() noexcept
    {
        shutdown();
        _engine = std::make_unique<ma_engine>();
        ma_engine_config config = ma_engine_config_init();
        auto maResult = ma_engine_init(&config, _engine.get());
        auto initResult = MiniaudioDetail::wrapExpected(maResult);
        if (!initResult)
        {
            return unexpected{ std::move(initResult).error() };
        }
        auto groupResult = MiniaudioSoundGroup::create(*_engine);
        if (!groupResult)
        {
            return unexpected{ std::move(groupResult).error() };
        }
        _soundGroup = std::move(groupResult).value();
        groupResult = MiniaudioSoundGroup::create(*_engine, MA_SOUND_FLAG_STREAM);
        if (!groupResult)
        {
            return unexpected{ std::move(groupResult).error() };
        }
        _musicGroup = std::move(groupResult).value();
        return {};
    }

    void AudioSystemImpl::shutdown() noexcept
    {
        _sounds.clear();
        _music.reset();
        _soundGroup.reset();
        _musicGroup.reset();
        if (_engine)
        {
            ma_engine_uninit(&*_engine);
            _engine.reset();
        }
    }

    void AudioSystemImpl::update() noexcept
    {
        auto itr = std::remove_if(_sounds.begin(), _sounds.end(), [](auto& elm) {
            return elm.miniaudio.atEnd();
        });
        _sounds.erase(itr, _sounds.end());
    }

    expected<std::reference_wrapper<MiniaudioSound>, std::string> AudioSystemImpl::createMiniaudioSound(const std::shared_ptr<Sound>& sound) noexcept
    {
        if (!_engine)
        {
            return unexpected<std::string>{"engine unitialized"};
        }
        auto soundGroupPtr = _soundGroup ? &*_soundGroup : nullptr;
        auto result = MiniaudioSound::create(sound->getImpl().getData(), *_engine, soundGroupPtr);
        if (!result)
        {
            return unexpected{ std::move(result).error() };
        }
        auto& elm = _sounds.emplace_back(std::move(result).value(), sound);
        return std::ref(elm.miniaudio);
    }

    expected<void, std::string> AudioSystemImpl::play(const std::shared_ptr<Sound>& sound) noexcept
    {
        auto result = createMiniaudioSound(sound);
        if (!result)
        {
            return unexpected{ std::move(result).error() };
        }
        auto& maSound = result.value().get();
        return maSound.start();
    }

    expected<void, std::string> AudioSystemImpl::play(const std::shared_ptr<Sound>& sound, const glm::vec3& pos) noexcept
    {
        auto result = createMiniaudioSound(sound);
        if (!result)
        {
            return unexpected{ std::move(result).error() };
        }
        auto& maSound = result.value().get();
        maSound.setPosition(pos);
        return maSound.start();
    }

    expected<void, std::string> AudioSystemImpl::play(const std::shared_ptr<Music>& music) noexcept
    {
        if (!_engine)
        {
            return unexpected<std::string>{"engine unitialized"};
        }
        auto musicGroupPtr = _musicGroup ? &*_musicGroup : nullptr;
        auto result = MiniaudioSound::create(music->getImpl().getData(), *_engine, musicGroupPtr);
        if (!result)
        {
            return unexpected{ std::move(result).error() };
        }
        _music = { std::move(result).value(), music };
        auto& maSound = _music->miniaudio;
        maSound.setLooping(true);
        return maSound.start();
    }

    float AudioSystemImpl::getVolume(AudioGroup group) const noexcept
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

    void AudioSystemImpl::setVolume(AudioGroup group, float v) noexcept
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

    void AudioSystemImpl::stopMusic() noexcept
    {
        _music.reset();
    }

    expected<void, std::string> AudioSystemImpl::pauseMusic() noexcept
    {
        if (!_music)
        {
            return unexpected<std::string>{ "music not initialized" };
        }
        if(_music->miniaudio.isPlaying())
        {
            auto stopResult = _music->miniaudio.stop();
            if (!stopResult)
            {
                return unexpected{ std::move(stopResult).error() };
            }
        }
        else
        {
            auto startResult = _music->miniaudio.start();
            if (!startResult)
            {
                return unexpected{ std::move(startResult).error() };
            }
        }

        return {};
    }

    MusicState AudioSystemImpl::getMusicState() const noexcept
    {
        if (!_music)
        {
            return MusicState::Stopped;
        }
        auto& audio = _music->miniaudio;
        return audio.isPlaying() ? MusicState::Playing : MusicState::Paused;
    }


    AudioSystem::AudioSystem() noexcept
        : _impl(std::make_unique<AudioSystemImpl>())
    {
    }

    AudioSystem::~AudioSystem() noexcept
    {
        // empty on purpose
    }

    const AudioSystemImpl& AudioSystem::getImpl() const noexcept
    {
        return *_impl;
    }

    AudioSystemImpl& AudioSystem::getImpl() noexcept
    {
        return *_impl;
    }
    
    expected<void, std::string> AudioSystem::play(const std::shared_ptr<Sound>& sound) noexcept
    {
        return _impl->play(sound);
    }

    expected<void, std::string> AudioSystem::play(const std::shared_ptr<Sound>& sound, const glm::vec3& pos) noexcept
    {
        return _impl->play(sound, pos);
    }

    expected<void, std::string> AudioSystem::play(const std::shared_ptr<Music>& music) noexcept
    {
        return _impl->play(music);
    }

    float AudioSystem::getVolume(AudioGroup group) const noexcept
    {
        return _impl->getVolume(group);
    }

    void AudioSystem::setVolume(AudioGroup group, float v) noexcept
    {
        _impl->setVolume(group, v);
    }

    void AudioSystem::stopMusic() noexcept
    {
        return _impl->stopMusic();
    }

    expected<void, std::string> AudioSystem::pauseMusic() noexcept
    {
        return _impl->pauseMusic();
    }

    MusicState AudioSystem::getMusicState() const noexcept
    {
        return _impl->getMusicState();
    }
}