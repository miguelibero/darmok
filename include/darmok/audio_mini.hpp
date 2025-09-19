#pragma once

#include <darmok/audio.hpp>

namespace darmok
{
    class IDataLoader;

    class DARMOK_EXPORT MiniaudioSoundLoader final : public ISoundLoader
    {
    public:
        MiniaudioSoundLoader(IDataLoader& dataLoader) noexcept;
        Result operator()(std::filesystem::path path) noexcept override;
    private:
        IDataLoader& _dataLoader;
    };

    class DARMOK_EXPORT MiniaudioMusicLoader final : public IMusicLoader
    {
    public:
        MiniaudioMusicLoader(IDataLoader& dataLoader) noexcept;
        Result operator()(std::filesystem::path path) noexcept override;
    private:
        IDataLoader& _dataLoader;
    };
}