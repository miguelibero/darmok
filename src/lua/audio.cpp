#include "audio.hpp"
#include <darmok/audio.hpp>

namespace darmok
{
    LuaAudioSystem::LuaAudioSystem(AudioSystem& audio) noexcept
        : _audio(audio)
    {
    }

    const AudioSystem& LuaAudioSystem::getReal() const noexcept
    {
        return _audio.get();
    }

    AudioSystem& LuaAudioSystem::getReal() noexcept
    {
        return _audio.get();
    }

    void LuaAudioSystem::play1(const std::shared_ptr<Sound>& sound) noexcept
    {
        getReal().play(sound);
    }

    void LuaAudioSystem::play2(const std::shared_ptr<Sound>& sound, const VarLuaTable<glm::vec3>& pos) noexcept
    {
        getReal().play(sound, LuaGlm::tableGet(pos));
    }

    void LuaAudioSystem::play3(const std::shared_ptr<Music>& music) noexcept
    {
        getReal().play(music);
    }

    float LuaAudioSystem::getSoundVolume() const
    {
        return getReal().getVolume(AudioGroup::Sound);
    }

    void LuaAudioSystem::setSoundVolume(float v)
    {
        return getReal().setVolume(AudioGroup::Sound, v);
    }

    float LuaAudioSystem::getMusicVolume() const
    {
        return getReal().getVolume(AudioGroup::Music);
    }

    void LuaAudioSystem::setMusicVolume(float v)
    {
        return getReal().setVolume(AudioGroup::Music, v);
    }

    void LuaAudioSystem::stopMusic()
    {
        return getReal().stopMusic();
    }

    void LuaAudioSystem::pauseMusic()
    {
        return getReal().pauseMusic();
    }

    MusicState LuaAudioSystem::getMusicState() const noexcept
    {
        return getReal().getMusicState();
    }

    bool LuaAudioSystem::getMusicPlaying() const noexcept
    {
        return getMusicState() == MusicState::Playing;
    }

    bool LuaAudioSystem::getMusicStopped() const noexcept
    {
        return getMusicState() == MusicState::Stopped;
    }

    bool LuaAudioSystem::getMusicPaused() const noexcept
    {
        return getMusicState() == MusicState::Paused;
    }

    void LuaAudioSystem::bind(sol::state_view& lua) noexcept
    {
        lua.new_enum<MusicState>("MusicState", {
            { "Stopped", MusicState::Stopped },
            { "Playing", MusicState::Playing },
            { "Paused", MusicState::Paused }
        });

        lua.new_usertype<Sound>("Sound", sol::no_constructor
        );

        lua.new_usertype<Music>("Music", sol::no_constructor
        );

        lua.new_usertype<LuaAudioSystem>("AudioSystem", sol::no_constructor,
            "play", sol::overload(
                &LuaAudioSystem::play1,
                &LuaAudioSystem::play2,
                &LuaAudioSystem::play3
            ),
			"sound_volume", sol::property(&LuaAudioSystem::getSoundVolume, &LuaAudioSystem::setSoundVolume),
            "music_volume", sol::property(&LuaAudioSystem::getMusicVolume, &LuaAudioSystem::setMusicVolume),
            "stop_music", &LuaAudioSystem::stopMusic,
            "pause_music", &LuaAudioSystem::pauseMusic,
			"music_state", sol::property(&LuaAudioSystem::getMusicState),
            "music_paused", sol::property(&LuaAudioSystem::getMusicPaused),
            "music_playing", sol::property(&LuaAudioSystem::getMusicPlaying),
            "music_stopped", sol::property(&LuaAudioSystem::getMusicStopped)
		);
    }
}