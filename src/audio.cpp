#include <darmok/audio.hpp>
#include <darmok/asset.hpp>

namespace darmok
{
    AudioSystem::AudioSystem() noexcept
    {
    }

    void AudioSystem::init(AssetContext& assets)
    {
        _soundLoader = assets.getSoundLoader();
        _musicLoader = assets.getMusicLoader();
        _player.init();
    }

    void AudioSystem::shutdown()
    {
        _player.shutdown();
    }

    bool AudioSystem::loadSound(std::string_view name, std::string_view loadName)
    {
        if (!_soundLoader)
        {
            return false;
        }
        if (loadName.empty())
        {
            loadName = name;
        }
        _sounds.emplace(name, (*_soundLoader)(loadName));
        return true;
    }

    bool AudioSystem::isSoundLoaded(std::string_view name) const noexcept
    {
        return _sounds.contains(std::string(name));
    }

    bool AudioSystem::loadMusic(std::string_view name, std::string_view loadName)
    {
        if (!_musicLoader)
        {
            return false;
        }
        if (loadName.empty())
        {
            loadName = name;
        }
        _music.emplace(name, (*_musicLoader)(loadName));
        return true;
    }

    bool AudioSystem::isMusicLoaded(std::string_view name) const noexcept
    {
        return _music.contains(std::string(name));
    }

    bool AudioSystem::playSound(std::string_view name) noexcept
    {
        auto itr = _sounds.find(std::string(name));
        if (itr == _sounds.end())
        {
            return false;
        }
        _player.play(itr->second);
        return true;
    }

    bool AudioSystem::playMusic(std::string_view name) noexcept
    {
        auto itr = _music.find(std::string(name));
        if (itr == _music.end())
        {
            return false;
        }
        _player.play(itr->second);
        return true;
    }

    bool AudioSystem::stopMusic()
    {
        return true;
    }

    bool AudioSystem::pauseMusic()
    {
        return true;
    }

    const std::string& AudioSystem::getRunningMusic() noexcept
    {
        return _runningMusic;
    }
}