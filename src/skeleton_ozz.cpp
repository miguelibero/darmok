#include "skeleton_ozz.hpp"
#include <darmok/skeleton_ozz.hpp>
#include <darmok/data.hpp>
#include <darmok/scene.hpp>
#include <darmok/math.hpp>
#include <darmok/easing.hpp>
#include <darmok/glm_serialize.hpp>
#include <ozz/base/io/archive.h>
#include <ozz/base/span.h>
#include <ozz/base/maths/simd_math.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <glm/gtx/quaternion.hpp>
#include <optional>
#include <sstream>

namespace darmok
{
    namespace OzzUtils
    {
        glm::mat4 convert(const ozz::math::Float4x4& v)
        {
            // convert from right-handed (ozz) to left-handed (bgfx)
            return Math::flipHandedness((const glm::mat4&)v);
        }

        template<typename T>
        expected<T, std::string> loadFromData(IDataLoader& loader, const std::filesystem::path& path) noexcept
        {
            auto dataResult = loader(path);
			if (!dataResult)
			{
                return unexpected<std::string>{dataResult.error()};
			}            
            DataOzzStream stream{ dataResult.value() };
            ozz::io::IArchive archive(&stream);
            if (!archive.TestTag<T>())
            {
                return unexpected<std::string>{"archive does not contain the type"};
            }
            T obj;
            archive >> obj;
            return obj;
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

    std::string Skeleton::toString() const noexcept
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

    std::string SkeletalAnimation::toString() const noexcept
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
        size_t pos = 0;
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

    OzzSkeletonLoader::Result OzzSkeletonLoader::operator()(std::filesystem::path path)
    {
        auto skelResult = OzzUtils::loadFromData<ozz::animation::Skeleton>(_dataLoader, path);
        if (!skelResult)
        {
            return unexpected<std::string>{skelResult.error()};
        }
        return std::make_shared<Skeleton>(
            std::make_unique<SkeletonImpl>(std::move(skelResult.value())));
    }

    OzzSkeletalAnimationLoader::OzzSkeletalAnimationLoader(IDataLoader& dataLoader) noexcept
        : _dataLoader(dataLoader)
    {
    }

    OzzSkeletalAnimationLoader::Result OzzSkeletalAnimationLoader::operator()(std::filesystem::path path)
    {
        auto anim = OzzUtils::loadFromData<ozz::animation::Animation>(_dataLoader, path);
        if (!anim)
        {
            return unexpected<std::string>{"archive doesn't contain an animation"};
        }
        return std::make_shared<SkeletalAnimation>(
            std::make_unique<SkeletalAnimationImpl>(std::move(anim.value())));
    }

    SkeletalAnimatorImpl::SkeletalAnimatorImpl(SkeletalAnimator& animator, const std::shared_ptr<Skeleton>& skeleton, const AnimationMap& animations, const Definition& def) noexcept
        : _animator(animator)
        , _skeleton(skeleton)
        , _speed(1.F)
        , _paused(false)
        , _def(def)
        , _animations(animations)
        , _blendPosition(0)
    {
        auto& skel = getOzz();
        _models.resize(skel.num_joints());

        ozz::animation::LocalToModelJob job;
        job.input = skel.joint_rest_poses();
        job.output = ozz::make_span(_models);
        job.skeleton = &skel;
        job.Run();
    }

    SkeletalAnimatorImpl::~SkeletalAnimatorImpl()
    {
        for (auto& listener : _listeners.copy())
        {
            listener.onAnimatorDestroyed(_animator);
        }
    }

    ozz::animation::Skeleton& SkeletalAnimatorImpl::getOzz() noexcept
    {
        return _skeleton->getImpl().getOzz();
    }

    const ozz::animation::Skeleton& SkeletalAnimatorImpl::getOzz() const noexcept
    {
        return _skeleton->getImpl().getOzz();
    }

    void SkeletalAnimatorImpl::addListener(std::unique_ptr<ISkeletalAnimatorListener>&& listener) noexcept
    {
        _listeners.insert(std::move(listener));
    }

    void SkeletalAnimatorImpl::addListener(ISkeletalAnimatorListener& listener) noexcept
    {
        _listeners.insert(listener);
    }

    bool SkeletalAnimatorImpl::removeListener(const ISkeletalAnimatorListener& listener) noexcept
    {
        return _listeners.erase(listener);
    }

    size_t SkeletalAnimatorImpl::removeListeners(const ISkeletalAnimatorListenerFilter& filter) noexcept
    {
        return _listeners.eraseIf(filter);
    }

    void SkeletalAnimatorImpl::setPlaybackSpeed(float speed) noexcept
    {
        _speed = speed;
    }

    float SkeletalAnimatorImpl::getPlaybackSpeed() const noexcept
    {
        return _speed;
    }

    void SkeletalAnimatorImpl::setBlendPosition(const glm::vec2& value) noexcept
    {
        _blendPosition = value;
    }

    const glm::vec2& SkeletalAnimatorImpl::getBlendPosition() const noexcept
    {
        return _blendPosition;
    }

    const SkeletalAnimatorImpl::Definition& SkeletalAnimatorImpl::getDefinition() const noexcept
    {
        return _def;
    }

    OptionalRef<const ISkeletalAnimatorState> SkeletalAnimatorImpl::getCurrentState() const noexcept
    {
        if (_transition)
        {
            return _transition->getCurrentOzzState();
        }
        if (!_state)
        {
            return nullptr;
        }
        return _state.value();
    }

    OptionalRef<SkeletalAnimatorImpl::State> SkeletalAnimatorImpl::getCurrentOzzState() noexcept
    {
        if (_transition)
        {
            return _transition->getCurrentOzzState();
        }
        if (!_state)
        {
            return nullptr;
        }
        return _state.value();
    }

    OptionalRef<const ISkeletalAnimatorTransition> SkeletalAnimatorImpl::getCurrentTransition() const noexcept
    {
        if (!_transition)
        {
            return nullptr;
        }
        return _transition.value();
    }

    OptionalRef<SkeletalAnimatorImpl::Transition> SkeletalAnimatorImpl::getCurrentOzzTransition() noexcept
    {
        if (!_transition)
        {
            return nullptr;
        }
        return _transition.value();
    }

    float SkeletalAnimatorImpl::getStateDuration(const std::string& name) const noexcept
    {
		if (auto stateDef = SkeletalAnimatorUtils::getState(_def, name))
		{
            const State state(getOzz(), *stateDef, (ISkeletalAnimationProvider&)*this);
            return state.getDuration();
		}
        return 0.F;

    }

    OzzSkeletalAnimatorAnimationState::OzzSkeletalAnimatorAnimationState(const ozz::animation::Skeleton& skel, const Definition& def, ISkeletalAnimationProvider& animations)
        : _def(def)
        , _normalizedTime(0.F)
        , _looped(false)
    {
        _animation = animations.getAnimation(def.name());
        _sampling.Resize(skel.num_joints());
        _locals.resize(skel.num_soa_joints());
    }

    OzzSkeletalAnimatorAnimationState::OzzSkeletalAnimatorAnimationState(OzzSkeletalAnimatorAnimationState&& other) noexcept
        : _def(std::move(other._def))
        , _normalizedTime(other._normalizedTime)
        , _locals(std::move(other._locals))
        , _looped(other._looped)
        , _animation(other._animation)
    {
        _sampling.Resize(other._sampling.max_soa_tracks() * 4 - 3);
        other._normalizedTime = -bx::kFloatInfinity;
        other._locals.clear();
        other._sampling.Invalidate();
        other._sampling.Resize(0);
        other._animation.reset();
    }

    ozz::animation::Animation& OzzSkeletalAnimatorAnimationState::getOzz() const noexcept
    {
        return _animation->getImpl().getOzz();
    }

    float OzzSkeletalAnimatorAnimationState::getNormalizedTime() const noexcept
    {
        return _normalizedTime;
    }

    void OzzSkeletalAnimatorAnimationState::setNormalizedTime(float normalizedTime) noexcept
    {
        _normalizedTime = std::fmodf(normalizedTime, 1.F);
    }

    float OzzSkeletalAnimatorAnimationState::getDuration() const noexcept
    {
        auto& anim = getOzz();
        return anim.duration() / _def.speed();
    }

    bool OzzSkeletalAnimatorAnimationState::hasFinished() const noexcept
    {
        return _looped && !_def.loop();
    }

    void OzzSkeletalAnimatorAnimationState::update(float deltaTime)
    {
        if (hasFinished())
        {
            return;
        }

        auto normDeltaTime = deltaTime / getDuration();
        auto normTime = _normalizedTime + normDeltaTime;
        _looped = normTime > 1.F;

        if (hasFinished())
        {
            return;
        }

        setNormalizedTime(normTime);

        ozz::animation::SamplingJob sampling;
        sampling.animation = &getOzz();
        sampling.context = &_sampling;
        sampling.ratio = _normalizedTime;
        sampling.output = ozz::make_span(_locals);
        if (!sampling.Run())
        {
            throw std::runtime_error("error in the sampling job");
        }
    }

    bool OzzSkeletalAnimatorAnimationState::hasLooped() const noexcept
    {
        return _looped;
    }

    const ozz::vector<ozz::math::SoaTransform>& OzzSkeletalAnimatorAnimationState::getLocals() const noexcept
    {
        return _locals;
    }

    glm::vec2 OzzSkeletalAnimatorAnimationState::getBlendPosition() const noexcept
    {
        return protobuf::convert(_def.blend_position());
    }

    OzzSkeletalAnimatorState::OzzSkeletalAnimatorState(const ozz::animation::Skeleton& skel, const Definition& def, ISkeletalAnimationProvider& animations) noexcept
        : _def(def)
        , _blendPos(0.F)
        , _oldBlendPos(0.F)
        , _normalizedTime(0.F)
        , _normalizedTweenTime(0.F)
        , _duration(0)
        , _looped(false)
        , _speed(_def.speed())
    {
        _locals.resize(skel.num_soa_joints());
        _animations.reserve(_def.animations_size());
        for (auto& animConfig : _def.animations())
        {
            _animations.emplace_back(skel, animConfig, animations);

        }
        _layers.resize(_animations.size());

        _duration = calcDuration();
    }

    float OzzSkeletalAnimatorState::calcDuration() const noexcept
    {
        if (_animations.empty())
        {
            return 0.F;
        }
        int f = 1;
        for (const auto& anim : _animations)
        {
            auto d = anim.getDuration();
            int decimalPlaces = std::ceil(-std::log10(d - std::floor(d)));
            f = std::max(f, (int)std::pow(10, decimalPlaces));
        }

        std::vector<int> scaled;
        for (const auto& anim : _animations)
        {
            scaled.push_back(static_cast<int>(std::round(anim.getDuration() * f)));
        }

        int d = scaled[0];
        for (size_t i = 1; i < scaled.size(); ++i)
        {
            d = std::lcm(d, scaled[i]);
        }

        return static_cast<float>(d) / f;
    }

    glm::vec2 OzzSkeletalAnimatorState::getBlendedPosition(float f) const noexcept
    {
        return _oldBlendPos + f * (_blendPos - _oldBlendPos);
    }

    void OzzSkeletalAnimatorState::setSpeed(float speed) noexcept
    {
        _speed = speed;
    }

    void OzzSkeletalAnimatorState::afterFinished() noexcept
    {
        _speed = _def.speed();
    }

    void OzzSkeletalAnimatorState::update(float deltaTime, const glm::vec2& blendPosition)
    {
        deltaTime *= _speed;
        for (auto& anim : _animations)
        {
            anim.update(deltaTime);
        }

        auto normDeltaTime = deltaTime / _duration;
        _normalizedTime += normDeltaTime;
        _looped = _normalizedTime > 1.F;
        _normalizedTime = std::fmodf(_normalizedTime, 1.F);

        if (_animations.size() <= 1)
        {
            return;
        }
        ozz::animation::BlendingJob blending;
        blending.layers = ozz::make_span(_layers);
        blending.output = ozz::make_span(_locals);
        blending.threshold = _def.threshold();

        if (_blendPos != blendPosition)
        {
            auto blendFactor = SkeletalAnimatorUtils::calcTween(_def.tween(), _normalizedTweenTime);
            _oldBlendPos = getBlendedPosition(blendFactor);
            _normalizedTweenTime = 0.F;
            _blendPos = blendPosition;
        }
        _normalizedTweenTime += deltaTime / _def.tween().duration();
        if (_normalizedTweenTime >= 1.F)
        {
            _normalizedTweenTime = 1.F;
        }
        auto blendFactor = SkeletalAnimatorUtils::calcTween(_def.tween(), _normalizedTweenTime);
        auto pos = getBlendedPosition(blendFactor);

        size_t i = 0;
        auto weights = SkeletalAnimatorUtils::calcBlendWeights(_def, pos);
        for (auto& anim : _animations)
        {
            if (anim.getBlendPosition() == glm::vec2(0))
            {
                // set blend position (0, 0) as rest pose
                blending.rest_pose = ozz::make_span(anim.getLocals());
            }
            if (anim.getBlendPosition() == pos)
            {
                // early exit since one of the animations is just in the blend position
                _locals = anim.getLocals();
                return;
            }
            if (i < weights.size())
            {
                _layers[i].weight = weights[i];
            }
            _layers[i].transform = ozz::make_span(anim.getLocals());
            ++i;
        }
        if (blending.rest_pose.empty())
        {
            blending.rest_pose = ozz::make_span(_layers[0].transform);
        }

        if (!blending.Run())
        {
            throw std::runtime_error("error in the blending job");
        }
    }

    std::string_view OzzSkeletalAnimatorState::getName() const noexcept
    {
        return _def.name();
    }

    const ozz::vector<ozz::math::SoaTransform>& OzzSkeletalAnimatorState::getLocals() const
    {
        if (_animations.size() == 1)
        {
            return _animations.front().getLocals();
        }
        return _locals;
    }

    bool OzzSkeletalAnimatorState::hasLooped() const noexcept
    {
        return _looped;
    }

    float OzzSkeletalAnimatorState::getDuration() const noexcept
    {
        return _duration * _speed;
    }

    float OzzSkeletalAnimatorState::getNormalizedTime() const noexcept
    {
        return _normalizedTime;
    }

    bool OzzSkeletalAnimatorState::hasFinished() const noexcept
    {
        for (auto& anim : _animations)
        {
            if (!anim.hasFinished())
            {
                return false;
            }
        }
        return true;
    }

    const std::string& OzzSkeletalAnimatorState::getNextState() const noexcept
    {
        return _def.next_state();
    }

    OzzSkeletalAnimatorTransition::OzzSkeletalAnimatorTransition(const Definition& def, State&& currentState, State&& previousState) noexcept
        : _def(def)
        , _currentState(std::move(currentState))
        , _previousState(std::move(previousState))
        , _locals(_previousState.getLocals())
        , _normalizedTime(0.F)
    {
    }

    float OzzSkeletalAnimatorTransition::getDuration() const noexcept
    {
        return _def.tween().duration();
    }

    float OzzSkeletalAnimatorTransition::getNormalizedTime() const noexcept
    {
        return _normalizedTime;
    }

    void OzzSkeletalAnimatorTransition::setNormalizedTime(float normalizedTime) noexcept
    {
        _normalizedTime = std::fmodf(normalizedTime, 1.F);
    }

    void OzzSkeletalAnimatorTransition::update(float deltaTime, const glm::vec2& blendPosition)
    {
        if (hasFinished())
        {
            _locals = _currentState.getLocals();
            return;
        }

        _previousState.update(deltaTime, blendPosition);
        auto currentStateDeltaTime = deltaTime;
        if (_normalizedTime == 0.F)
        {
            currentStateDeltaTime += _def.offset();
        }
        _currentState.update(currentStateDeltaTime, blendPosition);
        _normalizedTime += deltaTime / getDuration();
        auto v = SkeletalAnimatorUtils::calcTween(_def.tween(), _normalizedTime);

        std::array<ozz::animation::BlendingJob::Layer, 2> layers;
        layers[0].weight = 1.F - v;
        layers[0].transform = ozz::make_span(_previousState.getLocals());
        layers[1].weight = v;
        layers[1].transform = ozz::make_span(_currentState.getLocals());

        ozz::animation::BlendingJob blending;
        blending.layers = ozz::make_span(layers);
        blending.output = ozz::make_span(_locals);
        blending.threshold = bx::kFloatSmallest;
        blending.rest_pose = layers[0].transform;

        if (!blending.Run())
        {
            throw std::runtime_error("error in the blending job");
        }
    }

    bool OzzSkeletalAnimatorTransition::hasFinished() const noexcept
    {
        return _normalizedTime >= 1.F;
    }

    OzzSkeletalAnimatorTransition::State&& OzzSkeletalAnimatorTransition::finish() noexcept
    {
        return std::move(_currentState);
    }

    const ozz::vector<ozz::math::SoaTransform>& OzzSkeletalAnimatorTransition::getLocals() const noexcept
    {
        return _locals;
    }

    OzzSkeletalAnimatorState& OzzSkeletalAnimatorTransition::getPreviousOzzState() noexcept
    {
        return _previousState;
    }

    OzzSkeletalAnimatorState& OzzSkeletalAnimatorTransition::getCurrentOzzState() noexcept
    {
        return _currentState;
    }

    const OzzSkeletalAnimatorState& OzzSkeletalAnimatorTransition::getCurrentOzzState() const noexcept
    {
        return _currentState;
    }

    const OzzSkeletalAnimatorState& OzzSkeletalAnimatorTransition::getPreviousOzzState() const noexcept
    {
        return _previousState;
    }

    const ISkeletalAnimatorState& OzzSkeletalAnimatorTransition::getPreviousState() const noexcept
    {
        return _previousState;
    }

    const ISkeletalAnimatorState& OzzSkeletalAnimatorTransition::getCurrentState() const noexcept
    {
        return _currentState;
    }

    ISkeletalAnimatorState& OzzSkeletalAnimatorTransition::getPreviousState() noexcept
    {
        return _previousState;
    }

    ISkeletalAnimatorState& OzzSkeletalAnimatorTransition::getCurrentState() noexcept
    {
        return _currentState;
    }

    bool SkeletalAnimatorImpl::play(std::string_view name, float stateSpeed) noexcept
    {
        auto currState = getCurrentOzzState();
        if (currState && currState->getName() == name)
        {
            currState->setSpeed(stateSpeed);
            return false;
        }

        auto stateDef = SkeletalAnimatorUtils::getState(_def, name);
        if (!stateDef)
        {
            return false;
        }
        std::optional<State> prevState;
        if (_state)
        {
            prevState.emplace(std::move(_state.value()));
            _state.reset();
        }
        if (_transition)
        {
            prevState.emplace(_transition->finish());
            _transition.reset();
        }

        auto listeners = _listeners.copy();

        if (prevState)
        {
            auto transConfig = SkeletalAnimatorUtils::getTransition(_def, prevState->getName(), name);
            if (transConfig)
            {
                _transition.emplace(transConfig.value(),
                    State(getOzz(), *stateDef, *this),
                    std::move(prevState.value()));
                for (auto& listener : listeners)
                {
                    listener.onAnimatorStateStarted(_animator, _transition->getCurrentState().getName());
                }
                for (auto& listener : listeners)
                {
                    listener.onAnimatorTransitionStarted(_animator);
                }
                return true;
            }
        }
        if (_state && !_state->hasFinished())
        {
            for (auto& listener : listeners)
            {
                listener.onAnimatorStateFinished(_animator, _state->getName());
            }
            _state->afterFinished();
        }

        _state.emplace(getOzz(), *stateDef, *this);
        for (auto& listener : listeners)
        {
            listener.onAnimatorStateStarted(_animator, _state->getName());
        }

        return true;
    }

    void SkeletalAnimatorImpl::stop() noexcept
    {
        _state.reset();
    }

    void SkeletalAnimatorImpl::pause() noexcept
    {
        _paused = !_paused;
    }

    SkeletalAnimatorImpl::PlaybackState SkeletalAnimatorImpl::getPlaybackState() noexcept
    {
        if (!_state)
        {
            return PlaybackState::Stopped;
        }
        if (_paused)
        {
            return PlaybackState::Paused;
        }
        return PlaybackState::Playing;
    }

    std::shared_ptr<SkeletalAnimation> SkeletalAnimatorImpl::getAnimation(std::string_view name) noexcept
    {
        auto itr = _animations.find(std::string(name));
        if (itr == _animations.end())
        {
            return nullptr;
        }
        return itr->second;
    }

    SkeletalAnimator::~SkeletalAnimator()
    {
        // intentionally empty to be able to forward declare the impl
    }

    glm::mat4 SkeletalAnimator::getJointModelMatrix(const std::string& name) const noexcept
    {
        return _impl->getJointModelMatrix(name);
    }

    std::unordered_map<std::string, glm::mat4> SkeletalAnimator::getBoneModelMatrixes(const glm::vec3& dir) const noexcept
    {
        return _impl->getBoneModelMatrixes(dir);
    }

    void SkeletalAnimator::update(float deltaTime)
    {
        _impl->update(deltaTime);
    }

    glm::mat4 SkeletalAnimatorImpl::getJointModelMatrix(const std::string& joint) const noexcept
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

    std::unordered_map<std::string, glm::mat4> SkeletalAnimatorImpl::getBoneModelMatrixes(const glm::vec3& dir) const noexcept
    {
        auto& skel = _skeleton->getImpl().getOzz();
        auto numJoints = skel.num_joints();
        auto parents = skel.joint_parents();
        std::unordered_map<std::string, glm::mat4> bones;
        bones.reserve(numJoints);
        auto jointNames = skel.joint_names();
        for (int i = 0; i < numJoints; ++i)
        {
            auto parentId = parents[i];
            if (parentId == ozz::animation::Skeleton::kNoParent)
            {
                continue;
            }
            std::string_view name = jointNames[parentId];
            glm::vec3 parentPos = OzzUtils::convert(_models[parentId])[3];
            glm::vec3 childPos = OzzUtils::convert(_models[i])[3];
            auto diff = childPos - parentPos;
            const auto rot = glm::rotation(dir, glm::normalize(diff));
            const auto scale = glm::vec3(glm::length(diff));
            auto bone = Math::transform(parentPos, rot, scale);
            bones.emplace(name, bone);
        }
        return bones;
    }

    void SkeletalAnimatorImpl::update(float deltaTime)
    {
        deltaTime *= _speed;
        ozz::animation::LocalToModelJob ltm;

        if (_transition)
        {
            _transition->update(deltaTime, _blendPosition);
            ltm.input = ozz::make_span(_transition->getLocals());
        }
        else if (_state)
        {
            _state->update(deltaTime, _blendPosition);
            ltm.input = ozz::make_span(_state->getLocals());
        }
        else
        {
            return;
        }

        ltm.skeleton = &getOzz();
        ltm.output = ozz::make_span(_models);
        if (!ltm.Run())
        {
            throw std::runtime_error("error in the model job");
        }
        afterUpdate();
    }

    void SkeletalAnimatorImpl::afterUpdate() noexcept
    {
        auto listeners = _listeners.copy();
        if (_transition && _transition->hasFinished())
        {
            for (auto& listener : listeners)
            {
                listener.onAnimatorTransitionFinished(_animator);
            }
            for (auto& listener : listeners)
            {
                listener.onAnimatorStateFinished(_animator, _transition->getPreviousState().getName());
            }
            if (_state)
            {
                _state->afterFinished();
            }
            _state.emplace(_transition->finish());
            _transition.reset();
        }
        if (_state)
        {
            if (_state->hasLooped())
            {
                for (auto& listener : listeners)
                {
                    listener.onAnimatorStateLooped(_animator, _state->getName());
                }
            }
            if (_state->hasFinished())
            {
                for (auto& listener : listeners)
                {
                    listener.onAnimatorStateFinished(_animator, _state->getName());
                }
                _state->afterFinished();
                auto& nextState = _state->getNextState();
                if (!nextState.empty())
                {
                    play(nextState);
                }
            }
        }
    }

    SkeletalAnimator::SkeletalAnimator(const std::shared_ptr<Skeleton>& skel, const AnimationMap& anims, const Definition& def) noexcept
        : _impl(std::make_unique<SkeletalAnimatorImpl>(*this, skel, anims, def))
    {
    }

    SkeletalAnimator& SkeletalAnimator::addListener(std::unique_ptr<ISkeletalAnimatorListener>&& listener) noexcept
    {
        _impl->addListener(std::move(listener));
        return *this;
    }

    SkeletalAnimator& SkeletalAnimator::addListener(ISkeletalAnimatorListener& listener) noexcept
    {
        _impl->addListener(listener);
        return *this;
    }

    bool SkeletalAnimator::removeListener(const ISkeletalAnimatorListener& listener) noexcept
    {
        return _impl->removeListener(listener);
    }

    size_t SkeletalAnimator::removeListeners(const ISkeletalAnimatorListenerFilter& filter) noexcept
    {
        return _impl->removeListeners(filter);
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

    SkeletalAnimator& SkeletalAnimator::setBlendPosition(const glm::vec2& value) noexcept
    {
        _impl->setBlendPosition(value);
        return *this;
    }

    const glm::vec2& SkeletalAnimator::getBlendPosition() const noexcept
    {
        return _impl->getBlendPosition();
    }

    const SkeletalAnimator::Definition& SkeletalAnimator::getDefinition() const noexcept
    {
        return _impl->getDefinition();
    }

    OptionalRef<const ISkeletalAnimatorState> SkeletalAnimator::getCurrentState() const noexcept
    {
        return _impl->getCurrentState();
    }

    OptionalRef<const ISkeletalAnimatorTransition> SkeletalAnimator::getCurrentTransition() const noexcept
    {
        return _impl->getCurrentTransition();
    }

    float SkeletalAnimator::getStateDuration(const std::string& name) const noexcept
    {
        return _impl->getStateDuration(name);
    }

    bool SkeletalAnimator::play(std::string_view name, float stateSpeed) noexcept
    {
        return _impl->play(name, stateSpeed);
    }

    void SkeletalAnimator::stop() noexcept
    {
        _impl->stop();
    }

    void SkeletalAnimator::pause() noexcept
    {
        _impl->pause();
    }

    SkeletalAnimator::PlaybackState SkeletalAnimator::getPlaybackState() noexcept
    {
        return _impl->getPlaybackState();
    }
}