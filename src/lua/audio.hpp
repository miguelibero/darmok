#pragma once

#include <sol/sol.hpp>
#include "glm.hpp"
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
        LuaAudioSystem(AudioSystem& audio) noexcept;

        const AudioSystem& getReal() const noexcept;
        AudioSystem& getReal() noexcept;

        static void bind(sol::state_view& lua) noexcept;
    private:
        std::reference_wrapper<AudioSystem> _audio;

        void play1(const std::shared_ptr<Sound>& sound) noexcept;
        void play2(const std::shared_ptr<Sound>& sound, const VarLuaTable<glm::vec3>& pos) noexcept;
        void play3(const std::shared_ptr<Music>& music) noexcept;

        float getSoundVolume() const;
        void setSoundVolume(float v);

        float getMusicVolume() const;
        void setMusicVolume(float v);

        void stopMusic();
        void pauseMusic();

        MusicState getMusicState() const noexcept;
        bool getMusicPlaying() const noexcept;
        bool getMusicStopped() const noexcept;
        bool getMusicPaused() const noexcept;
    };
}