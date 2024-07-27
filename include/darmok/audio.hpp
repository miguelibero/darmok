#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <bx/bx.h>
#include <glm/glm.hpp>

namespace darmok
{
    class SoundImpl;

    class DARMOK_EXPORT Sound final
    {
        Sound(std::unique_ptr<SoundImpl>&& impl);
        ~Sound();
    private:
        std::unique_ptr<SoundImpl> _impl;
    };

    class MusicImpl;

    class DARMOK_EXPORT Music final
    {
    public:
        Music(std::unique_ptr<MusicImpl>&& impl);
        ~Music();
    private:
        std::unique_ptr<MusicImpl> _impl;
    };

    class AudioPlayerImpl;

    class DARMOK_EXPORT AudioPlayer final
    {
    public:
        AudioPlayer() noexcept;
        ~AudioPlayer() noexcept;

        void init();
        void shutdown();
        bool play(const std::shared_ptr<Sound>& sound) noexcept;
        bool play(const std::shared_ptr<Sound>& sound, const glm::vec3& pos) noexcept;
        bool play(const std::shared_ptr<Music>& music) noexcept;
        bool stopMusic();
        bool pauseMusic();
        std::shared_ptr<Music> getRunningMusic() noexcept;

    private:
        std::unique_ptr<AudioPlayerImpl> _impl;
    };

    class DARMOK_EXPORT BX_NO_VTABLE ISoundLoader
    {
    public:
        using result_type = std::shared_ptr<Sound>;
        virtual ~ISoundLoader() = default;
        virtual result_type operator()(std::string_view name) = 0;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IMusicLoader
    {
    public:
        using result_type = std::shared_ptr<Music>;
        virtual ~IMusicLoader() = default;
        virtual result_type operator()(std::string_view name) = 0;
    };

    class AssetContext;

    class DARMOK_EXPORT AudioSystem final
    {
    public:
        AudioSystem() noexcept;

        void init(AssetContext& assets);
        void shutdown();

        bool loadSound(std::string_view name, std::string_view loadName = "");
        bool isSoundLoaded(std::string_view name) const noexcept;
        bool loadMusic(std::string_view name, std::string_view loadName = "");
        bool isMusicLoaded(std::string_view name) const noexcept;

        bool playSound(std::string_view name) noexcept;
        bool playMusic(std::string_view name) noexcept;
        bool stopMusic();
        bool pauseMusic();

        const std::string& getRunningMusic() noexcept;

    private:
        AudioPlayer _player;
        OptionalRef<ISoundLoader> _soundLoader;
        OptionalRef<IMusicLoader> _musicLoader;
        std::unordered_map<std::string, std::shared_ptr<Sound>> _sounds;
        std::unordered_map<std::string, std::shared_ptr<Music>> _music;
        std::string _runningMusic;
    };
}