#include "skeleton_ozz.hpp"
#include <darmok/data.hpp>
#include <darmok/scene.hpp>
#include <darmok/math.hpp>
#include <ozz/base/io/archive.h>
#include <ozz/base/span.h>
#include <ozz/base/maths/simd_math.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <optional>

namespace darmok
{
    SkeletonImpl::SkeletonImpl(ozz::animation::Skeleton&& skel) noexcept
        : _skel(std::move(skel))
    {
    }

    ozz::animation::Skeleton& SkeletonImpl::getOzz() noexcept
    {
        return _skel;
    }

    const ozz::animation::Skeleton& SkeletonImpl::getOzz() const noexcept
    {
        return _skel;
    }

    Skeleton::Skeleton(std::unique_ptr<SkeletonImpl>&& impl) noexcept
        : _impl(std::move(impl))
    {
    }

    Skeleton::~Skeleton()
    {
        // intentionally empty to be able to forward declare the impl
    }

    SkeletonImpl& Skeleton::getImpl()
    {
        return *_impl;
    }

    const SkeletonImpl& Skeleton::getImpl() const
    {
        return *_impl;
    }

    SkeletalAnimationImpl::SkeletalAnimationImpl(ozz::animation::Animation&& anim) noexcept
        : _anim(std::move(anim))
    {
    }

    ozz::animation::Animation& SkeletalAnimationImpl::getOzz() noexcept
    {
        return _anim;
    }

    const ozz::animation::Animation& SkeletalAnimationImpl::getOzz() const noexcept
    {
        return _anim;
    }

    SkeletalAnimation::SkeletalAnimation(std::unique_ptr<SkeletalAnimationImpl>&& impl) noexcept
        : _impl(std::move(impl))
    {
    }

    SkeletalAnimation::~SkeletalAnimation()
    {
        // intentionally empty to be able to forward declare the impl
    }


    SkeletalAnimationImpl& SkeletalAnimation::getImpl()
    {
        return *_impl;
    }

    const SkeletalAnimationImpl& SkeletalAnimation::getImpl() const
    {
        return *_impl;
    }

    std::string_view SkeletalAnimation::getName() const noexcept
    {
        return _impl->getOzz().name();
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

    SkeletalAnimationControllerImpl::SkeletalAnimationControllerImpl(const std::shared_ptr<Skeleton>& skeleton, const std::vector<std::shared_ptr<SkeletalAnimation>>& animations) noexcept
        : _skeleton(skeleton)
        , _animations(animations)
        , _timeRatio(0.F)
        , _playbackSpeed(1.F)
        , _play(true)
        , _loop(true)
    {
        auto& skel = _skeleton->getImpl().getOzz();
        _locals.resize(skel.num_soa_joints());
        _models.resize(skel.num_joints());
        _sampling.Resize(skel.num_joints());
    }

    SkeletalAnimationController::~SkeletalAnimationController()
    {
        // intentionally empty to be able to forward declare the impl
    }

    void SkeletalAnimationControllerImpl::addAnimation(const std::shared_ptr<SkeletalAnimation>& anim) noexcept
    {
        _animations.push_back(anim);
    }

    bool SkeletalAnimationControllerImpl::playAnimation(std::string_view name, bool loop)
    {
        for (auto anim : _animations)
        {
            if (anim->getName() == name)
            {
                _currentAnimation = anim;
                _loop = loop;
                _timeRatio = 0;
                _play = true;
                return true;
            }
        }
        return false;
    }

    void SkeletalAnimationControllerImpl::setTimeRatio(float ratio) noexcept
    {
        if (_loop)
        {
            _timeRatio = ratio - floorf(ratio);
        }
        else
        {
            _timeRatio = Math::clamp(0.f, ratio, 1.f);
        }
    }

    bool SkeletalAnimationControllerImpl::update(float deltaTime) noexcept
    {

        if (_currentAnimation == nullptr)
        {
            return false;
        }
        auto& anim = _currentAnimation->getImpl().getOzz();

        {
            float newTime = _timeRatio;
            if (_play)
            {
                newTime = _timeRatio + deltaTime * _playbackSpeed / anim.duration();
            }
            setTimeRatio(newTime);
        }

        ozz::animation::SamplingJob sampling;
        sampling.animation = &anim;
        sampling.context = &_sampling;
        sampling.ratio = _timeRatio;
        sampling.output = ozz::make_span(_locals);
        if (!sampling.Run())
        {
            return false;
        }

        ozz::animation::LocalToModelJob ltm_job;
        ltm_job.skeleton = &_skeleton->getImpl().getOzz();
        ltm_job.input = ozz::make_span(_locals);
        ltm_job.output = ozz::make_span(_models);
        if (!ltm_job.Run())
        {
            return false;
        }

        return true;
    }

    SkeletalAnimationController::SkeletalAnimationController(const std::shared_ptr<Skeleton>& skel, const std::vector<std::shared_ptr<SkeletalAnimation>>& animations) noexcept
        : _impl(std::make_unique<SkeletalAnimationControllerImpl>(skel, animations))
    {
    }

    SkeletalAnimationController& SkeletalAnimationController::addAnimation(const std::shared_ptr<SkeletalAnimation>& anim) noexcept
    {
        _impl->addAnimation(anim);
        return *this;
    }

    bool SkeletalAnimationController::playAnimation(std::string_view name) noexcept
    {
        return _impl->playAnimation(name);
    }
}