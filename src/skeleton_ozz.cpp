#include "skeleton_ozz.hpp"
#include <darmok/data.hpp>
#include <darmok/scene.hpp>
#include <darmok/math.hpp>
#include <ozz/base/io/archive.h>
#include <ozz/base/span.h>
#include <ozz/base/maths/simd_math.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/blending_job.h>
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

    SkeletalAnimatorImpl::SkeletalAnimatorImpl(const std::shared_ptr<Skeleton>& skeleton, const Config& config) noexcept
        : _skeleton(skeleton)
        , _speed(1.F)
        , _config(config)
    {
        auto& skel = getOzz();
        _models.resize(skel.num_joints());

        ozz::animation::LocalToModelJob job;
        job.input = skel.joint_rest_poses();
        job.output = ozz::make_span(_models);
        job.skeleton = &skel;
        job.Run();
    }

    ozz::animation::Skeleton& SkeletalAnimatorImpl::getOzz() noexcept
    {
        return _skeleton->getImpl().getOzz();
    }

    const ozz::animation::Skeleton& SkeletalAnimatorImpl::getOzz() const noexcept
    {
        return _skeleton->getImpl().getOzz();
    }

    void SkeletalAnimatorImpl::setPlaybackSpeed(float speed) noexcept
    {
        _speed;
    }

    float SkeletalAnimatorImpl::getPlaybackSpeed() const noexcept
    {
        return _speed;
    }

    const SkeletalAnimatorImpl::Config& SkeletalAnimatorImpl::getConfig() const noexcept
    {
        return _config;
    }

    OptionalRef<ISkeletalAnimatorState> SkeletalAnimatorImpl::getCurrentState() noexcept
    {
        if (!_state)
        {
            return nullptr;
        }
        return _state.value();
    }

    OptionalRef<ISkeletalAnimatorTransition> SkeletalAnimatorImpl::getCurrentTransition() noexcept
    {
        if (!_transition)
        {
            return nullptr;
        }
        return _transition.value();
    }

    OzzSkeletalAnimatorState::OzzSkeletalAnimatorState(const ozz::animation::Skeleton& skel, const Config& config) noexcept
        : _config(config)
        , _normalizedTime(0.F)
    {
        _sampling.Resize(skel.num_joints());
        _locals.resize(skel.num_soa_joints());
    }

    std::string_view OzzSkeletalAnimatorState::getName() const noexcept
    {
        return _config.name.empty() ? _config.motion->getName() : _config.name;
    }

    ozz::animation::Animation& OzzSkeletalAnimatorState::getOzz() noexcept
    {
        return _config.motion->getImpl().getOzz();
    }

    float OzzSkeletalAnimatorState::getNormalizedTime() const noexcept
    {
        return _normalizedTime;
    }

    void OzzSkeletalAnimatorState::setNormalizedTime(float normalizedTime) noexcept
    {
        _normalizedTime = std::fmodf(normalizedTime, 1.F);
    }

    bool OzzSkeletalAnimatorState::update(float deltaTime) noexcept
    {
        auto& anim = getOzz();

        setNormalizedTime(_normalizedTime + (deltaTime / anim.duration()));

        ozz::animation::SamplingJob sampling;
        sampling.animation = &anim;
        sampling.context = &_sampling;
        sampling.ratio = _normalizedTime;
        sampling.output = ozz::make_span(_locals);
        return sampling.Run();
    }

    const ozz::vector<ozz::math::SoaTransform>& OzzSkeletalAnimatorState::getLocals() const noexcept
    {
        return _locals;
    }

    OzzSkeletalAnimatorTransition::OzzSkeletalAnimatorTransition(const ozz::animation::Skeleton& skel, const State::Config& newConfig, State&& previousState)
        : _currentState(skel, newConfig)
    {
        // _previousState.emplace(std::move(previousState));
    }

    bool OzzSkeletalAnimatorTransition::update(float deltaTime) noexcept
    {
        // setNormalizedTime(_normalizedTime + (deltaTime / anim.duration()));

        // update states

        // blend state locals
        return false;
    }

    float OzzSkeletalAnimatorTransition::getNormalizedTime() const noexcept
    {
        return _normalizedTime;
    }

    void OzzSkeletalAnimatorTransition::setNormalizedTime(float normalizedTime) noexcept
    {
        _normalizedTime = std::fmodf(normalizedTime, 1.F);
    }

    const ozz::vector<ozz::math::SoaTransform>& OzzSkeletalAnimatorTransition::getLocals() const noexcept
    {
        return _locals;
    }

    OptionalRef<ISkeletalAnimatorState> OzzSkeletalAnimatorTransition::getPreviousState() noexcept
    {
        if (!_previousState)
        {
            return nullptr;
        }
        return _previousState.value();
    }

    ISkeletalAnimatorState& OzzSkeletalAnimatorTransition::getCurrentState() noexcept
    {
        return _currentState;
    }

    bool SkeletalAnimatorImpl::play(std::string_view name, float normalizedTime) noexcept
    {
        if (_transition && _transition->getCurrentState().getName() == name)
        {
            return false;
        }
        auto config = _config.getState(name);
        auto setTime = normalizedTime >= 0.F && normalizedTime <= 1.F;
        if (!config)
        {
            return false;
        }
        if (_state)
        {
            _transition.emplace(getOzz(), config.value(), std::move(_state.value()));
            _state.reset();
            if (setTime)
            {
                _transition->setNormalizedTime(normalizedTime);
            }
        }
        else
        {
            _state.emplace(getOzz(), config.value());
            if (setTime)
            {
                _state->setNormalizedTime(normalizedTime);
            }
        }
        return true;
    }

    SkeletalAnimator::~SkeletalAnimator()
    {
        // intentionally empty to be able to forward declare the impl
    }

    glm::mat4 SkeletalAnimator::getModelMatrix(const std::string& boneName) const noexcept
    {
        return _impl->getModelMatrix(boneName);
    }

    std::vector<glm::mat4> SkeletalAnimator::getBoneMatrixes(const glm::vec3& dir) const noexcept
    {
        return _impl->getBoneMatrixes(dir);
    }

    void SkeletalAnimator::update(float deltaTime) noexcept
    {
        _impl->update(deltaTime);
    }

    glm::mat4 SkeletalAnimatorImpl::getModelMatrix(const std::string& joint) const noexcept
    {
        if (_skeleton == nullptr)
        {
            return glm::mat4(1);
        }
        auto jointNames = getOzz().joint_names();
        for (size_t i = 0; i < jointNames.size() && i < _models.size(); i++)
        {
            if (jointNames[i] == joint)
            {
                return OzzUtils::convert(_models[i]);
            }
        }
        return glm::mat4(1);
    }

    std::vector<glm::mat4> SkeletalAnimatorImpl::getBoneMatrixes(const glm::vec3& dir) const noexcept
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

    bool SkeletalAnimatorImpl::update(float deltaTime) noexcept
    {
        deltaTime *= _speed;
        ozz::animation::LocalToModelJob ltm;

        if (_transition)
        {
            _transition->update(deltaTime);
            ltm.input = ozz::make_span(_transition->getLocals());
        }
        else if (_state)
        {
            _state->update(deltaTime);
            ltm.input = ozz::make_span(_state->getLocals());
        }
        else
        {
            return false;
        }

        ltm.skeleton = &getOzz();
        ltm.output = ozz::make_span(_models);
        return ltm.Run();
    }

    SkeletalAnimator::SkeletalAnimator(const std::shared_ptr<Skeleton>& skel, const Config& config) noexcept
        : _impl(std::make_unique<SkeletalAnimatorImpl>(skel, config))
    {
    }

    SkeletalAnimator& SkeletalAnimator::setPlaybackSpeed(float speed) noexcept
    {
        _impl->setPlaybackSpeed(speed);
        return *this;
    }

    float SkeletalAnimator::getPlaybackSpeed() const noexcept
    {
        return _impl->getPlaybackSpeed();
    }

    const SkeletalAnimator::Config& SkeletalAnimator::getConfig() const noexcept
    {
        return _impl->getConfig();
    }

    OptionalRef<ISkeletalAnimatorState> SkeletalAnimator::getCurrentState() noexcept
    {
        return _impl->getCurrentState();
    }

    OptionalRef<ISkeletalAnimatorTransition> SkeletalAnimator::getCurrentTransition() noexcept
    {
        return _impl->getCurrentTransition();
    }

    bool SkeletalAnimator::play(std::string_view name, float normalizedTime) noexcept
    {
        return _impl->play(name, normalizedTime);
    }
}