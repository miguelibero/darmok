#pragma once

#include "lua/lua.hpp"
#include "lua/glm.hpp"
#include <memory>
#include <darmok/audio_fwd.hpp>

namespace darmok
{
    class AudioSystem;
    class Sound;
    class Music;

    class LuaAudioSystem final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        static void play1(AudioSystem& audio, const std::shared_ptr<Sound>& sound) noexcept;
        static void play2(AudioSystem& audio, const std::shared_ptr<Sound>& sound, const VarLuaTable<glm::vec3>& pos) noexcept;
        static void play3(AudioSystem& audio, const std::shared_ptr<Music>& music) noexcept;

        static float getSoundVolume(const AudioSystem& audio);
        static void setSoundVolume(AudioSystem& audio, float v);

        static float getMusicVolume(const AudioSystem& audio);
        static void setMusicVolume(AudioSystem& audio, float v);

        static bool getMusicPlaying(const AudioSystem& audio) noexcept;
        static bool getMusicStopped(const AudioSystem& audio) noexcept;
        static bool getMusicPaused(const AudioSystem& audio) noexcept;
    };
}