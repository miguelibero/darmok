#include "detail/skeleton_ozz.hpp"
#include <darmok/skeleton_ozz.hpp>
#include <darmok/data.hpp>
#include <darmok/scene.hpp>
#include <darmok/scene_serialize.hpp>
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

    Skeleton::Skeleton(std::unique_ptr<SkeletonImpl> impl) noexcept
        : _impl{ std::move(impl) }
    {
    }

    Skeleton::~Skeleton() = default;

    std::string Skeleton::toString() const noexcept
    {
        auto& skel = _impl->getOzz();
        return "Skeleton " + std::to_string(skel.num_joints()) + " joints";
    }

    SkeletonImpl::SkeletonImpl(ozz::animation::Skeleton skel) noexcept
        : _skel{ std::move(skel) }
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

    SkeletalAnimation::SkeletalAnimation(std::unique_ptr<SkeletalAnimationImpl> impl) noexcept
        : _impl{ std::move(impl) }
    {
    }

    SkeletalAnimation::~SkeletalAnimation() = default;

    std::string SkeletalAnimation::toString() const noexcept
    {
        auto& anim = _impl->getOzz();

        std::stringstream ss;
        ss << "SkeletalAnimation " << anim.name();
        return ss.str();
    }

    SkeletalAnimationImpl::SkeletalAnimationImpl(ozz::animation::Animation anim) noexcept
        : _anim{ std::move(anim) }
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
        : _data{ data }
        , _pos{ 0 }
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

    OzzSkeletonLoader::Result OzzSkeletonLoader::operator()(std::filesystem::path path) noexcept
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

    OzzSkeletalAnimationLoader::Result OzzSkeletalAnimationLoader::operator()(std::filesystem::path path) noexcept
    {
        auto anim = OzzUtils::loadFromData<ozz::animation::Animation>(_dataLoader, path);
        if (!anim)
        {
            return unexpected<std::string>{"archive doesn't contain an animation"};
        }
        return std::make_shared<SkeletalAnimation>(
            std::make_unique<SkeletalAnimationImpl>(std::move(anim.value())));
    }

    SkeletalAnimatorImpl::SkeletalAnimatorImpl(SkeletalAnimator& animator) noexcept
        : _animator{ animator }
        , _speed{ 1.f }
        , _paused{ false }
        , _blendPosition{ 0 }
    {
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

    void SkeletalAnimatorImpl::addListener(std::unique_ptr<ISkeletalAnimatorListener> listener) noexcept
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
        auto& anims = const_cast<ISkeletalAnimationProvider&>(static_cast<const ISkeletalAnimationProvider&>(*this));
        ConstSkeletalAnimatorDefinitionWrapper anim{ _def };
        if (auto stateDef = anim.getState(name))
        {
            if (auto state = State::create(getOzz(), *stateDef, anims))
            {
                return state->getDuration();
            }
        }
        return 0.f;

    }

    std::optional<OzzSkeletalAnimatorAnimationState> OzzSkeletalAnimatorAnimationState::create(const ozz::animation::Skeleton& skel, const Definition& def, ISkeletalAnimationProvider& anims)
    {
        if (auto anim = anims.getAnimation(def.name()))
        {
            return OzzSkeletalAnimatorAnimationState{ skel, def, anim };
        }
        return std::nullopt;
    }

    OzzSkeletalAnimatorAnimationState::OzzSkeletalAnimatorAnimationState(const ozz::animation::Skeleton& skel, const Definition& def, const std::shared_ptr<SkeletalAnimation>& anim)
        : _def{ def }
        , _normalizedTime{ 0.f }
        , _looped{ false }
        , _animation{ anim }
    {
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
        _normalizedTime = std::fmodf(normalizedTime, 1.f);
        _normalizedTime = std::fmodf(normalizedTime, 1.f);
    }

    float OzzSkeletalAnimatorAnimationState::getDuration() const noexcept
    {
        auto& anim = getOzz();
        auto speed = _def.speed();
        speed = speed == 0.f ? 1.f : speed;
        return anim.duration() / speed;
    }

    bool OzzSkeletalAnimatorAnimationState::hasFinished() const noexcept
    {
        return _looped && !_def.loop();
    }

    expected<void, std::string> OzzSkeletalAnimatorAnimationState::update(float deltaTime) noexcept
    {
        if (hasFinished())
        {
            return {};
        }

        auto normDeltaTime = deltaTime / getDuration();
        auto normTime = _normalizedTime + normDeltaTime;
        _looped = normTime > 1.f;

        if (hasFinished())
        {
            return {};
        }

        setNormalizedTime(normTime);

        ozz::animation::SamplingJob sampling;
        sampling.animation = &getOzz();
        sampling.context = &_sampling;
        sampling.ratio = _normalizedTime;
        sampling.output = ozz::make_span(_locals);
        if (!sampling.Run())
        {
            return unexpected<std::string>{ "error in the sampling job" };
        }

        return {};
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
        return convert<glm::vec2>(_def.blend_position());
    }

    OzzSkeletalAnimatorState::OzzSkeletalAnimatorState(const ozz::animation::Skeleton& skel, const Definition& def, std::vector<AnimationState> states) noexcept
        : _def{ def }
        , _animationStates{ std::move(states) }
        , _blendPos{ 0.f }
        , _oldBlendPos{ 0.f }
        , _normalizedTime{ 0.f }
        , _normalizedTweenTime{ 0.f }
        , _duration{ 0 }
        , _looped{ false }
        , _speed{ _def.speed() }
    {
        _locals.resize(skel.num_soa_joints());
        _layers.resize(_animationStates.size());
        _duration = calcDuration();
    }

    std::optional<OzzSkeletalAnimatorState> OzzSkeletalAnimatorState::create(const ozz::animation::Skeleton& skel, const Definition& def, ISkeletalAnimationProvider& animations) noexcept
    {
        std::vector<AnimationState> states;
        states.reserve(def.animations_size());
        for (auto& animDef : def.animations())
        {
            if (auto animState = AnimationState::create(skel, animDef, animations))
            {
                states.push_back(std::move(*animState));
            }
        }
        if (states.empty())
        {
            return std::nullopt;
        }
        return OzzSkeletalAnimatorState{ skel, def, std::move(states) };
    }

    float OzzSkeletalAnimatorState::calcDuration() const noexcept
    {
        if (_animationStates.empty())
        {
            return 0.f;
        }
        int f = 1;
        for (const auto& anim : _animationStates)
        {
            auto d = anim.getDuration();
            int decimalPlaces = std::ceil(-std::log10(d - std::floor(d)));
            f = std::max(f, (int)std::pow(10, decimalPlaces));
        }

        std::vector<int> scaled;
        for (const auto& anim : _animationStates)
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

    expected<void, std::string> OzzSkeletalAnimatorState::update(float deltaTime, const glm::vec2& blendPosition) noexcept
    {
        deltaTime *= _speed;
        for (auto& anim : _animationStates)
        {
            auto result = anim.update(deltaTime);
            if (!result)
            {
                return result;
            }
        }

        auto normDeltaTime = deltaTime / _duration;
        _normalizedTime += normDeltaTime;
        _looped = _normalizedTime > 1.f;
        _normalizedTime = std::fmodf(_normalizedTime, 1.f);

        if (_animationStates.size() <= 1)
        {
            return {};
        }
        ozz::animation::BlendingJob blending;
        blending.layers = ozz::make_span(_layers);
        blending.output = ozz::make_span(_locals);
        blending.threshold = _def.threshold();

        ConstSkeletalAnimatorTweenDefinitionWrapper tween{ _def.tween() };
        if (_blendPos != blendPosition)
        {
            auto blendFactor = tween.calcTween(_normalizedTweenTime);
            _oldBlendPos = getBlendedPosition(blendFactor);
            _normalizedTweenTime = 0.f;
            _blendPos = blendPosition;
        }
        _normalizedTweenTime += deltaTime / _def.tween().duration();
        if (_normalizedTweenTime >= 1.f)
        {
            _normalizedTweenTime = 1.f;
        }
        auto blendFactor = tween.calcTween(_normalizedTweenTime);
        auto pos = getBlendedPosition(blendFactor);

        size_t i = 0;
        ConstSkeletalAnimatorStateDefinitionWrapper state{ _def };
        auto weights = state.calcBlendWeights(pos);
        for (auto& anim : _animationStates)
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
                return {};
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
            return unexpected<std::string>{"error in the blending job"};
        }
        return {};
    }

    std::string_view OzzSkeletalAnimatorState::getName() const noexcept
    {
        return _def.name();
    }

    const ozz::vector<ozz::math::SoaTransform>& OzzSkeletalAnimatorState::getLocals() const noexcept
    {
        if (_animationStates.size() == 1)
        {
            return _animationStates.front().getLocals();
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
        for (auto& anim : _animationStates)
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

    OzzSkeletalAnimatorTransition::OzzSkeletalAnimatorTransition(const Definition& def, State currentState, State previousState) noexcept
        : _def{ def }
        , _currentState{ std::move(currentState) }
        , _previousState{ std::move(previousState) }
        , _locals{ _previousState.getLocals() }
        , _normalizedTime{ 0.f }
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
        _normalizedTime = std::fmodf(normalizedTime, 1.f);
    }

    expected<void, std::string> OzzSkeletalAnimatorTransition::update(float deltaTime, const glm::vec2& blendPosition) noexcept
    {
        if (hasFinished())
        {
            _locals = _currentState.getLocals();
            return {};
        }

        auto updateResult = _previousState.update(deltaTime, blendPosition);
        if (!updateResult)
        {
            return updateResult;
        }
        auto currentStateDeltaTime = deltaTime;
        if (_normalizedTime == 0.f)
        {
            currentStateDeltaTime += _def.offset();
        }
        auto result = _currentState.update(currentStateDeltaTime, blendPosition);
        if (!result)
        {
            return {};
        }
        _normalizedTime += deltaTime / getDuration();
        ConstSkeletalAnimatorTweenDefinitionWrapper tween{ _def.tween() };
        auto v = tween.calcTween(_normalizedTime);

        std::array<ozz::animation::BlendingJob::Layer, 2> layers;
        layers[0].weight = 1.f - v;
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
            return unexpected<std::string>{ "error in the blending job" };
        }
        return {};
    }

    bool OzzSkeletalAnimatorTransition::hasFinished() const noexcept
    {
        return _normalizedTime >= 1.f;
    }

    OzzSkeletalAnimatorTransition::State OzzSkeletalAnimatorTransition::finish() noexcept
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

        ConstSkeletalAnimatorDefinitionWrapper anim{ _def };
        auto stateDef = anim.getState(name);
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
            ConstSkeletalAnimatorDefinitionWrapper anim{ _def };
            auto transConfig = anim.getTransition(prevState->getName(), name);
            if (transConfig)
            {
                auto state = State::create(getOzz(), *stateDef, *this);
                if (!state)
                {
                    return false;
                }
                _transition.emplace(transConfig.value(), std::move(*state), std::move(prevState.value()));
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

        auto state = State::create(getOzz(), *stateDef, *this);
        if (!state)
        {
            return false;
        }
        _state = std::move(state);
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

    void SkeletalAnimatorImpl::reset() noexcept
    {
        stop();
		_speed = 1.f;
        _paused = false;
		_blendPosition = glm::vec2(0);
        _transition.reset();
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

    void SkeletalAnimatorImpl::load(std::shared_ptr<Skeleton> skeleton, AnimationMap animations, Definition def) noexcept
    {
        reset();

		_skeleton = std::move(skeleton);
		_animations = std::move(animations);
        _def = std::move(def);

        auto& skel = getOzz();
        _models.resize(skel.num_joints());
        ozz::animation::LocalToModelJob job;
        job.input = skel.joint_rest_poses();
        job.output = ozz::make_span(_models);
        job.skeleton = &skel;
        job.Run();
    }

    expected<void, std::string> SkeletalAnimatorImpl::load(const Definition& def, IComponentLoadContext& ctxt) noexcept
    {
        auto& assets = ctxt.getAssets();

        auto skelResult = assets.getSkeletonLoader()(def.skeleton_path());
        if (!skelResult)
        {
			return unexpected{ skelResult.error() };
        }
        ConstSkeletalAnimatorDefinitionWrapper anim{ def };
        auto animations = anim.loadAnimations(assets.getSkeletalAnimationLoader());

		load(skelResult.value(), std::move(animations), def);

        return {};
    }

    SkeletalAnimator::~SkeletalAnimator() = default;

    glm::mat4 SkeletalAnimator::getJointModelMatrix(const std::string& name) const noexcept
    {
        return _impl->getJointModelMatrix(name);
    }

    std::unordered_map<std::string, glm::mat4> SkeletalAnimator::getJointModelMatrixes(const glm::vec3& dir) const noexcept
    {
        return _impl->getJointModelMatrixes(dir);
    }

    expected<void, std::string> SkeletalAnimator::update(float deltaTime) noexcept
    {
        return _impl->update(deltaTime);
    }

    expected<void, std::string> SkeletalAnimator::load(const Definition& def, IComponentLoadContext& ctxt) noexcept
    {
		return _impl->load(def, ctxt);
    }

    SkeletalAnimator::Definition SkeletalAnimator::createDefinition() noexcept
    {
        return {};
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

    std::unordered_map<std::string, glm::mat4> SkeletalAnimatorImpl::getJointModelMatrixes(const glm::vec3& dir) const noexcept
    {
        auto& skel = _skeleton->getImpl().getOzz();
        auto numJoints = skel.num_joints();
        auto parents = skel.joint_parents();
        std::unordered_map<std::string, glm::mat4> matrixes;
        matrixes.reserve(numJoints);
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
            auto mat = Math::transform(parentPos, rot, scale);
            matrixes.emplace(name, mat);
        }
        return matrixes;
    }

    expected<void, std::string> SkeletalAnimatorImpl::update(float deltaTime) noexcept
    {
        deltaTime *= _speed;
        ozz::animation::LocalToModelJob ltm;

        if (_transition)
        {
            auto result = _transition->update(deltaTime, _blendPosition);
            if (!result)
            {
                return result;
            }
            ltm.input = ozz::make_span(_transition->getLocals());
        }
        else if (_state)
        {
            auto result = _state->update(deltaTime, _blendPosition);
            if (!result)
            {
                return result;
            }
            ltm.input = ozz::make_span(_state->getLocals());
        }
        else
        {
            return {};
        }

        ltm.skeleton = &getOzz();
        ltm.output = ozz::make_span(_models);
        if (!ltm.Run())
        {
            return unexpected<std::string>{ "error in the model job" };
        }
        afterUpdate();
        return {};
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

    SkeletalAnimator::SkeletalAnimator() noexcept
        : _impl{ std::make_unique<SkeletalAnimatorImpl>(*this) }
    {
    }

    SkeletalAnimator::SkeletalAnimator(std::shared_ptr<Skeleton> skel, AnimationMap anims, Definition def) noexcept
        : _impl{ std::make_unique<SkeletalAnimatorImpl>(*this) }
    {
		_impl->load(std::move(skel), std::move(anims), def);
    }

    SkeletalAnimator& SkeletalAnimator::addListener(std::unique_ptr<ISkeletalAnimatorListener> listener) noexcept
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