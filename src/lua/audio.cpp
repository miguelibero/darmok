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

    bool LuaAudioSystem::loadSound(const std::string& name, const std::string& loadName)
    {
        return getReal().loadSound(name, loadName);
    }

    bool LuaAudioSystem::isSoundLoaded(const std::string& name) const noexcept
    {
        return getReal().isSoundLoaded(name);
    }

    bool LuaAudioSystem::loadMusic(const std::string& name, const std::string& loadName)
    {
        return getReal().loadMusic(name, loadName);
    }

    bool LuaAudioSystem::isMusicLoaded(const std::string& name) const noexcept
    {
        return getReal().isMusicLoaded(name);
    }

    bool LuaAudioSystem::playSound(const std::string& name) noexcept
    {
        return getReal().playSound(name);
    }

    bool LuaAudioSystem::playMusic(const std::string& name) noexcept
    {
        return getReal().playMusic(name);
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

    const std::string& LuaAudioSystem::getRunningMusic() noexcept
    {
        return getReal().getRunningMusic();
    }

    static void LuaAudioSystem::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<LuaAudioSystem>("AudioSystem", sol::no_constructor,
            "load_sound", &LuaAudioSystem::loadSound,
            "is_sound_loaded", &LuaAudioSystem::isSoundLoaded,
            "load_music", &LuaAudioSystem::loadMusic,
            "is_music_loaded", &LuaAudioSystem::isMusicLoaded,
            "play_sound", &LuaAudioSystem::playSound,
            "play_music", &LuaAudioSystem::playMusic,
			"sound_volume", sol::property(&LuaAudioSystem::getSoundVolume, &LuaAudioSystem::setSoundVolume),
            "music_volume", sol::property(&LuaAudioSystem::getMusicVolume, &LuaAudioSystem::setMusicVolume),
            "stop_music", &LuaAudioSystem::stopMusic,
            "pause_music", &LuaAudioSystem::pauseMusic,
			"running_music", sol::property(&LuaAudioSystem::getRunningMusic)
		);
    }
}