#pragma once

#include <sol/sol.hpp>

namespace darmok
{
    class AudioSystem;

    class LuaAudioSystem final
    {
    public:
        LuaAudioSystem(AudioSystem& audio) noexcept;

        const AudioSystem& getReal() const noexcept;
        AudioSystem& getReal() noexcept;

        static void bind(sol::state_view& lua) noexcept;
    private:
        std::reference_wrapper<AudioSystem> _audio;

        bool loadSound(const std::string& name, const std::string& loadName = "");
        bool isSoundLoaded(const std::string& name) const noexcept;
        bool loadMusic(const std::string& name, const std::string& loadName = "");
        bool isMusicLoaded(const std::string& name) const noexcept;

        bool playSound(const std::string& name) noexcept;
        bool playMusic(const std::string& name) noexcept;

        float getSoundVolume() const;
        void setSoundVolume(float v);

        float getMusicVolume() const;
        void setMusicVolume(float v);

        void stopMusic();
        void pauseMusic();

        const std::string& getRunningMusic() noexcept;
    }
}