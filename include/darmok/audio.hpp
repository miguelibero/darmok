#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <darmok/audio_fwd.hpp>
#include <darmok/loader.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <bx/bx.h>
#include <darmok/glm.hpp>

namespace darmok
{
    class SoundImpl;

    class DARMOK_EXPORT Sound final
    {
    public:
        Sound(std::unique_ptr<SoundImpl>&& impl);
        ~Sound();
        float getDuration() const;
        SoundImpl& getImpl() noexcept;
        const SoundImpl& getImpl() const noexcept;
    private:
        std::unique_ptr<SoundImpl> _impl;
    };

    class MusicImpl;

    class DARMOK_EXPORT Music final
    {
    public:
        Music(std::unique_ptr<MusicImpl>&& impl);
        ~Music();
        float getDuration() const;
        MusicImpl& getImpl() noexcept;
        const MusicImpl& getImpl() const noexcept;
    private:
        std::unique_ptr<MusicImpl> _impl;
    };

    class AudioSystemImpl;

    class DARMOK_EXPORT AudioSystem final
    {
    public:
        AudioSystem() noexcept;
        ~AudioSystem() noexcept;
        AudioSystem(const AudioSystem&) = delete;
        AudioSystem(AudioSystem&&) = delete;
        AudioSystem& operator=(const AudioSystem&) = delete;
        AudioSystem& operator=(AudioSystem&&) = delete;

        const AudioSystemImpl& getImpl() const noexcept;
        AudioSystemImpl& getImpl() noexcept;

        void play(const std::shared_ptr<Sound>& sound) noexcept;
        void play(const std::shared_ptr<Sound>& sound, const glm::vec3& pos) noexcept;
        void play(const std::shared_ptr<Music>& music) noexcept;

        float getVolume(AudioGroup group) const;
        void setVolume(AudioGroup group, float v);

        void stopMusic();
        void pauseMusic();
        MusicState getMusicState() const noexcept;

    private:
        std::unique_ptr<AudioSystemImpl> _impl;
    };

    class DARMOK_EXPORT BX_NO_VTABLE ISoundLoader : public ILoader<Sound>{};
    class DARMOK_EXPORT BX_NO_VTABLE IMusicLoader : public ILoader<Music>{};
}