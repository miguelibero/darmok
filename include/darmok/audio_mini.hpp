#pragma once

#include <darmok/audio.hpp>

namespace darmok
{
    class IDataLoader;

    class DARMOK_EXPORT MiniaudioSoundLoader final : public ISoundLoader
    {
    public:
        MiniaudioSoundLoader(IDataLoader& dataLoader);
        Result operator()(std::filesystem::path path) override;
    private:
        IDataLoader& _dataLoader;
    };

    class DARMOK_EXPORT MiniaudioMusicLoader final : public IMusicLoader
    {
    public:
        MiniaudioMusicLoader(IDataLoader& dataLoader);
        Result operator()(std::filesystem::path path) override;
    private:
        IDataLoader& _dataLoader;
    };
}