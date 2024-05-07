#include "skeleton_ozz.hpp"
#include <darmok/data.hpp>
#include <ozz/base/io/archive.h>
#include <optional>

namespace darmok
{
    SkeletonImpl::SkeletonImpl(ozz::animation::Skeleton&& skel) noexcept
        : _skel(std::move(skel))
    {
    }

    SkeletalAnimationImpl::SkeletalAnimationImpl(ozz::animation::Animation&& anim) noexcept
        : _anim(std::move(anim))
    {
    }

    Skeleton::Skeleton(std::unique_ptr<SkeletonImpl>&& impl) noexcept
        : _impl(std::move(impl))
    {
    }

    SkeletalAnimation::SkeletalAnimation(std::unique_ptr<SkeletalAnimationImpl>&& impl) noexcept
        : _impl(std::move(impl))
    {
    }

    DataOzzStream::DataOzzStream(const DataView& data) noexcept
        : _data(data)
        , _pos(0)
    {
    }

    bool DataOzzStream::opened() const noexcept
    {
        return true;
    }

    size_t DataOzzStream::Read(void* buffer, size_t size) noexcept
    {
        auto view = _data.view(_pos, size);
        size = view.size();
        std::memcpy(buffer, view.ptr(), size);
        _pos += size;
        return size;
    }

    size_t DataOzzStream::Write(const void* buffer, size_t size) noexcept
    {
        auto view = _data.view(_pos, size);
        size = view.size();
        std::memcpy(const_cast<void*>(view.ptr()), buffer, size);
        _pos += size;
        return size;
    }

    int DataOzzStream::Seek(int offset, Origin origin) noexcept
    {
        size_t pos;
        switch (origin) {
        case kCurrent:
            pos = _pos;
            break;
        case kEnd:
            pos = _data.size();
            break;
        case kSet:
            pos = 0;
            break;
        default:
            return -1;
        }
        pos += offset;

        if (pos < 0 || pos >= _data.size())
        {
            return -1;
        }
        _pos = pos;
        return 0;
    }

    int DataOzzStream::Tell() const noexcept
    {
        return _pos;
    }

    size_t DataOzzStream::Size() const noexcept
    {
        return _data.size();
    }

    OzzSkeletonLoader::OzzSkeletonLoader(IDataLoader& dataLoader) noexcept
        : _dataLoader(dataLoader)
    {
    }

    template<typename T>
    static std::optional<T> loadOzzObjectFromData(IDataLoader& loader, std::string_view name) noexcept
    {
        auto data = loader(name);
        DataOzzStream stream(data.view());
        ozz::io::IArchive archive(&stream);
        if (!archive.TestTag<T>())
        {
            return std::nullopt;
        }
        T obj;
        archive >> obj;
        return obj;
    }

    std::shared_ptr<Skeleton> OzzSkeletonLoader::operator()(std::string_view name)
    {
        auto skel = loadOzzObjectFromData<ozz::animation::Skeleton>(_dataLoader, name);
        if (!skel)
        {
            throw std::runtime_error("archive doesn't contain a skeleton");
        }
        return std::make_shared<Skeleton>(
            std::make_unique<SkeletonImpl>(std::move(skel.value())));
    }

    OzzSkeletalAnimationLoader::OzzSkeletalAnimationLoader(IDataLoader& dataLoader) noexcept
        : _dataLoader(dataLoader)
    {
    }

    std::shared_ptr<SkeletalAnimation> OzzSkeletalAnimationLoader::operator()(std::string_view name)
    {
        auto anim = loadOzzObjectFromData<ozz::animation::Animation>(_dataLoader, name);
        if (!anim)
        {
            throw std::runtime_error("archive doesn't contain an animation");
        }
        return std::make_shared<SkeletalAnimation>(
            std::make_unique<SkeletalAnimationImpl>(std::move(anim.value())));
    }
}