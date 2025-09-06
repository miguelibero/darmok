#include "lua/audio.hpp"
#include <darmok/audio.hpp>

namespace darmok
{
    void LuaAudioSystem::play1(AudioSystem& audio, const std::shared_ptr<Sound>& sound) noexcept
    {
        audio.play(sound);
    }

    void LuaAudioSystem::play2(AudioSystem& audio, const std::shared_ptr<Sound>& sound, const VarLuaTable<glm::vec3>& pos) noexcept
    {
        audio.play(sound, LuaGlm::tableGet(pos));
    }

    void LuaAudioSystem::play3(AudioSystem& audio, const std::shared_ptr<Music>& music) noexcept
    {
        audio.play(music);
    }

    float LuaAudioSystem::getSoundVolume(const AudioSystem& audio)
    {
        return audio.getVolume(AudioGroup::Sound);
    }

    void LuaAudioSystem::setSoundVolume(AudioSystem& audio, float v)
    {
        return audio.setVolume(AudioGroup::Sound, v);
    }

    float LuaAudioSystem::getMusicVolume(const AudioSystem& audio)
    {
        return audio.getVolume(AudioGroup::Music);
    }

    void LuaAudioSystem::setMusicVolume(AudioSystem& audio, float v)
    {
        return audio.setVolume(AudioGroup::Music, v);
    }

    bool LuaAudioSystem::getMusicPlaying(const AudioSystem& audio) noexcept
    {
        return audio.getMusicState() == MusicState::Playing;
    }

    bool LuaAudioSystem::getMusicStopped(const AudioSystem& audio) noexcept
    {
        return audio.getMusicState() == MusicState::Stopped;
    }

    bool LuaAudioSystem::getMusicPaused(const AudioSystem& audio) noexcept
    {
        return audio.getMusicState() == MusicState::Paused;
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

        lua.new_usertype<AudioSystem>("AudioSystem", sol::no_constructor,
            "play", sol::overload(
                &LuaAudioSystem::play1,
                &LuaAudioSystem::play2,
                &LuaAudioSystem::play3
            ),
			"sound_volume", sol::property(&LuaAudioSystem::getSoundVolume, &LuaAudioSystem::setSoundVolume),
            "music_volume", sol::property(&LuaAudioSystem::getMusicVolume, &LuaAudioSystem::setMusicVolume),
            "stop_music", &AudioSystem::stopMusic,
            "pause_music", &AudioSystem::pauseMusic,
			"music_state", sol::property(&AudioSystem::getMusicState),
            "music_paused", sol::property(&LuaAudioSystem::getMusicPaused),
            "music_playing", sol::property(&LuaAudioSystem::getMusicPlaying),
            "music_stopped", sol::property(&LuaAudioSystem::getMusicStopped)
		);
    }
}