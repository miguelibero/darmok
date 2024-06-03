#include "skeleton_ozz.hpp"
#include <darmok/data.hpp>
#include <darmok/scene.hpp>
#include <darmok/math.hpp>
#include <ozz/base/io/archive.h>
#include <ozz/base/span.h>
#include <ozz/base/maths/simd_math.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <glm/gtx/quaternion.hpp>
#include <optional>
#include <sstream>


namespace darmok
{
    struct OzzUtils final
    {
        static glm::mat4 convert(const ozz::math::Float4x4& v)
        {
            // convert from right-handed (ozz) to left-handed (bgfx)
            return Math::flipHandedness((const glm::mat4&)v);
        }
    };

    Skeleton::Skeleton(std::unique_ptr<SkeletonImpl>&& impl) noexcept
        : _impl(std::move(impl))
    {
    }

    Skeleton::~Skeleton()
    {
        // intentionally empty to be able to forward declare the impl
    }

    std::string Skeleton::to_string() const noexcept
    {
        auto& skel = _impl->getOzz();

        std::stringstream ss;
        ss << "Skeleton " << skel.num_joints() << "joints";
        return ss.str();
    }

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

    SkeletalAnimation::SkeletalAnimation(std::unique_ptr<SkeletalAnimationImpl>&& impl) noexcept
        : _impl(std::move(impl))
    {
    }

    SkeletalAnimation::~SkeletalAnimation()
    {
        // intentionally empty to be able to forward declare the impl
    }

    std::string SkeletalAnimation::to_string() const noexcept
    {
        auto& anim = _impl->getOzz();

        std::stringstream ss;
        ss << "SkeletalAnimation " << anim.name();
        return ss.str();
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

    float SkeletalAnimationImpl::getDuration() const noexcept
    {
        return _anim.duration();
    }

    std::string_view SkeletalAnimation::getName() const noexcept
    {
        return _impl->getOzz().name();
    }

    float SkeletalAnimation::getDuration() const noexcept
    {
        return _impl->getDuration();
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

        ozz::animation::LocalToModelJob job;
        job.input = skel.joint_rest_poses();
        job.output = ozz::make_span(_models);
        job.skeleton = &skel;
        job.Run();
    }

    SkeletalAnimationController::~SkeletalAnimationController()
    {
        // intentionally empty to be able to forward declare the impl
    }

    glm::mat4 SkeletalAnimationController::getModelMatrix(const std::string& boneName) const noexcept
    {
        return _impl->getModelMatrix(boneName);
    }

    std::vector<glm::mat4> SkeletalAnimationController::getBoneMatrixes(const glm::vec3& dir) const noexcept
    {
        return _impl->getBoneMatrixes(dir);
    }

    SkeletalAnimationController& SkeletalAnimationController::setPlaybackSpeed(float speed) noexcept
    {
        _impl->setPlaybackSpeed(speed);
        return *this;
    }

    float SkeletalAnimationController::getPlaybackSpeed() const noexcept
    {
        return _impl->getPlaybackSpeed();
    }

    void SkeletalAnimationController::update(float deltaTime) noexcept
    {
        _impl->update(deltaTime);
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
        _timeRatio = std::fmodf(ratio, 1.F);
    }

    void SkeletalAnimationControllerImpl::setPlaybackSpeed(float speed) noexcept
    {
        _playbackSpeed = speed;
    }

    float SkeletalAnimationControllerImpl::getPlaybackSpeed() const noexcept
    {
        return _playbackSpeed;
    }

    glm::mat4 SkeletalAnimationControllerImpl::getModelMatrix(const std::string& joint) const noexcept
    {
        if (_skeleton == nullptr)
        {
            return glm::mat4(1);
        }
        auto jointNames = _skeleton->getImpl().getOzz().joint_names();
        for (size_t i = 0; i < jointNames.size() && i < _models.size(); i++)
        {
            if (jointNames[i] == joint)
            {
                return OzzUtils::convert(_models[i]);
            }
        }
        return glm::mat4(1);
    }

    std::vector<glm::mat4> SkeletalAnimationControllerImpl::getBoneMatrixes(const glm::vec3& dir) const noexcept
    {
        auto& skel = _skeleton->getImpl().getOzz();
        auto numJoints = skel.num_joints();
        auto parents = skel.joint_parents();
        std::vector<glm::mat4> bones;
        bones.reserve(numJoints);
        for (int i = 0; i < numJoints; ++i)
        {
            auto parentId = parents[i];
            if (parentId == ozz::animation::Skeleton::kNoParent)
            {
                continue;
            }
            glm::vec3 parentPos = OzzUtils::convert(_models[parentId])[3];
            glm::vec3 childPos = OzzUtils::convert(_models[i])[3];
            auto diff = childPos - parentPos;
            auto rot = glm::rotation(dir, glm::normalize(diff));
            auto scale = glm::vec3(glm::length(diff));
            auto bone = Math::transform(parentPos, rot, scale);
            bones.push_back(bone);
        }
        return bones;
    }

    bool SkeletalAnimationControllerImpl::update(float deltaTime) noexcept
    {
        if (_currentAnimation == nullptr)
        {
            return false;
        }
        auto& anim = _currentAnimation->getImpl().getOzz();

        if (_play)
        {
            auto timeRatio = _timeRatio + (deltaTime * _playbackSpeed / anim.duration());
            if (timeRatio > 1.F)
            {
                if (!_loop)
                {
                    _play = false;
                    return false;
                }
            }
            setTimeRatio(timeRatio);
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

        ozz::animation::LocalToModelJob ltm;
        ltm.skeleton = &_skeleton->getImpl().getOzz();
        ltm.input = ozz::make_span(_locals);
        ltm.output = ozz::make_span(_models);
        if (!ltm.Run())
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

    bool SkeletalAnimationController::playAnimation(std::string_view name, bool loop) noexcept
    {
        return _impl->playAnimation(name, loop);
    }
}