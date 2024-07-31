#pragma once

#include <darmok/audio.hpp>

namespace darmok
{
    class IDataLoader;

    class DARMOK_EXPORT MiniaudioSoundLoader final : public ISoundLoader
    {
    public:
        MiniaudioSoundLoader(IDataLoader& dataLoader);
        std::shared_ptr<Sound> operator()(std::string_view name) override;
    private:
        IDataLoader& _dataLoader;
    };

    class DARMOK_EXPORT MiniaudioMusicLoader final : public IMusicLoader
    {
    public:
        MiniaudioMusicLoader(IDataLoader& dataLoader);
        std::shared_ptr<Music> operator()(std::string_view name) override;
    private:
        IDataLoader& _dataLoader;
    };
}