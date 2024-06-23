#include "skeleton_ozz.hpp"
#include <darmok/skeleton_ozz.hpp>
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

    SkeletalAnimatorImpl::SkeletalAnimatorImpl(SkeletalAnimator& animator, const std::shared_ptr<Skeleton>& skeleton, const Config& config) noexcept
        : _animator(animator)
        , _skeleton(skeleton)
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

    void SkeletalAnimatorImpl::addListener(ISkeletalAnimatorListener& listener) noexcept
    {
        _listeners.emplace(listener);
    }

    bool SkeletalAnimatorImpl::removeListener(ISkeletalAnimatorListener& listener) noexcept
    {
        auto ptr = &listener;
        auto itr = std::find_if(_listeners.begin(), _listeners.end(), [ptr](auto& ref) { return ref.ptr() == ptr; });
        if (itr == _listeners.end())
        {
            return false;
        }
        _listeners.erase(itr);
        return true;
    }

    void SkeletalAnimatorImpl::setPlaybackSpeed(float speed) noexcept
    {
        _speed;
    }

    float SkeletalAnimatorImpl::getPlaybackSpeed() const noexcept
    {
        return _speed;
    }

    void SkeletalAnimatorImpl::setBlendPosition(const glm::vec2& value) noexcept
    {
        _blendPosition = value;
    }

    const SkeletalAnimatorImpl::Config& SkeletalAnimatorImpl::getConfig() const noexcept
    {
        return _config;
    }

    OptionalRef<const ISkeletalAnimatorState> SkeletalAnimatorImpl::getCurrentState() const noexcept
    {
        if (_transition)
        {
            return _transition->getCurrentState();
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

    OzzSkeletalAnimatorAnimationState::OzzSkeletalAnimatorAnimationState(const ozz::animation::Skeleton& skel, const Config& config) noexcept
        : _config(config)
        , _normalizedTime(0.F)
        , _lastUpdateLooped(false)
    {
        _sampling.Resize(skel.num_joints());
        _locals.resize(skel.num_soa_joints());
    }

    OzzSkeletalAnimatorAnimationState::OzzSkeletalAnimatorAnimationState(OzzSkeletalAnimatorAnimationState&& other) noexcept
        : _config(std::move(other._config))
        , _normalizedTime(other._normalizedTime)
        , _locals(std::move(other._locals))
        , _lastUpdateLooped(other._lastUpdateLooped)
    {
        _sampling.Resize(other._sampling.max_soa_tracks() * 4 - 3);
        other._normalizedTime = -bx::kFloatInfinity;
        other._locals.clear();
        other._sampling.Invalidate();
        other._sampling.Resize(0);
    }

    ozz::animation::Animation& OzzSkeletalAnimatorAnimationState::getOzz() const noexcept
    {
        return _config.animation->getImpl().getOzz();
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
        return anim.duration() / _config.speed;
    }

    void OzzSkeletalAnimatorAnimationState::update(float deltaTime)
    {
        auto normDeltaTime = deltaTime / getDuration();
        auto normTime = _normalizedTime + normDeltaTime;
        _lastUpdateLooped = normTime > 1.F;
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

    bool OzzSkeletalAnimatorAnimationState::hasLoopedDuringLastUpdate() const noexcept
    {
        return _lastUpdateLooped;
    }

    const ozz::vector<ozz::math::SoaTransform>& OzzSkeletalAnimatorAnimationState::getLocals() const noexcept
    {
        return _locals;
    }

    const glm::vec2& OzzSkeletalAnimatorAnimationState::getBlendPosition() const noexcept
    {
        return _config.blendPosition;
    }

    OzzSkeletalAnimatorState::OzzSkeletalAnimatorState(const ozz::animation::Skeleton& skel, const Config& config) noexcept
        : _config(config)
        , _blendPos(0.F)
        , _oldBlendPos(0.F)
        , _tween(_config.tween.create())
    {
        _locals.resize(skel.num_soa_joints());
        _animations.reserve(_config.animations.size());
        for (auto& animConfig : _config.animations)
        {
            _animations.emplace_back(skel, animConfig);
        }
        _layers.resize(_animations.size());
    }

    glm::vec2 OzzSkeletalAnimatorState::getBlendedPosition(float f) const noexcept
    {
        return _oldBlendPos + f * (_blendPos - _oldBlendPos);
    }

    void OzzSkeletalAnimatorState::update(float deltaTime, const glm::vec2& blendPosition)
    {
        for (auto& anim : _animations)
        {
            anim.update(deltaTime);
        }
        if (_animations.size() <= 1)
        {
            return;
        }
        ozz::animation::BlendingJob blending;
        blending.layers = ozz::make_span(_layers);
        blending.output = ozz::make_span(_locals);
        blending.threshold = _config.threshold;

        if (_blendPos != blendPosition)
        {
            _oldBlendPos = getBlendedPosition(_tween.peek());
            _tween.seek(0.F);
            _blendPos = blendPosition;
        }
        auto blendFactor = _tween.step(int(deltaTime * 1000));
        auto pos = getBlendedPosition(blendFactor);

        size_t i = 0;
        auto weights = _config.calcBlendWeights(pos);
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
        return _config.name;
    }

    const ozz::vector<ozz::math::SoaTransform>& OzzSkeletalAnimatorState::getLocals() const
    {
        if (_animations.size() == 1)
        {
            return _animations.front().getLocals();
        }
        return _locals;
    }

    OzzSkeletalAnimatorTransition::OzzSkeletalAnimatorTransition(const Config& config, State&& currentState, State&& previousState) noexcept
        : _config(config)
        , _currentState(std::move(currentState))
        , _previousState(std::move(previousState))
        , _locals(_previousState.getLocals())
        , _normalizedTime(0.F)
        , _tween(_config.tween.create())
    {
    }

    float OzzSkeletalAnimatorTransition::getDuration() const noexcept
    {
        return _config.tween.duration;
    }

    float OzzSkeletalAnimatorTransition::getNormalizedTime() const noexcept
    {
        return _normalizedTime;
    }

    void OzzSkeletalAnimatorTransition::setNormalizedTime(float normalizedTime) noexcept
    {
        _normalizedTime = normalizedTime;
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
            currentStateDeltaTime += _config.offset;
        }
        _currentState.update(currentStateDeltaTime, blendPosition);
        _normalizedTime += deltaTime / getDuration();
        auto v = _tween.step(int(deltaTime * 1000));

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

    const ISkeletalAnimatorState& OzzSkeletalAnimatorTransition::getPreviousState() const noexcept
    {
        return _previousState;
    }

    const ISkeletalAnimatorState& OzzSkeletalAnimatorTransition::getCurrentState() const noexcept
    {
        return _currentState;
    }

    bool SkeletalAnimatorImpl::play(std::string_view name) noexcept
    {
        auto currState = getCurrentState();
        if (currState && currState->getName() == name)
        {
            return false;
        }
        auto stateConfig = _config.getState(name);
        if (!stateConfig)
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

        if (prevState)
        {
            auto transConfig = _config.getTransition(prevState->getName(), name);
            if (transConfig)
            {
                _transition.emplace(transConfig.value(), State(getOzz(), stateConfig.value()), std::move(prevState.value()));
                for (auto& listener : _listeners)
                {
                    listener->onAnimatorStateStarted(_animator, _transition->getCurrentState().getName());
                }
                for (auto& listener : _listeners)
                {
                    listener->onAnimatorTransitionStarted(_animator);
                }
                return true;
            }
        }
        if (_state)
        {
            for (auto& listener : _listeners)
            {
                listener->onAnimatorStateFinished(_animator, _state->getName());
            }
        }

        _state.emplace(getOzz(), stateConfig.value());
        for (auto& listener : _listeners)
        {
            listener->onAnimatorStateStarted(_animator, _state->getName());
        }

        return true;
    }

    SkeletalAnimator::~SkeletalAnimator()
    {
        // intentionally empty to be able to forward declare the impl
    }

    glm::mat4 SkeletalAnimator::getJointModelMatrix(const std::string& name) const noexcept
    {
        return _impl->getJointModelMatrix(name);
    }

    std::vector<glm::mat4> SkeletalAnimator::getBoneModelMatrixes(const glm::vec3& dir) const noexcept
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

    std::vector<glm::mat4> SkeletalAnimatorImpl::getBoneModelMatrixes(const glm::vec3& dir) const noexcept
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
        if (_transition && _transition->hasFinished())
        {
            for (auto& listener : _listeners)
            {
                listener->onAnimatorTransitionFinished(_animator);
            }
            for (auto& listener : _listeners)
            {
                listener->onAnimatorStateFinished(_animator, _transition->getPreviousState().getName());
            }
            _state.emplace(_transition->finish());
            _transition.reset();
        }
    }

    SkeletalAnimator::SkeletalAnimator(const std::shared_ptr<Skeleton>& skel, const Config& config) noexcept
        : _impl(std::make_unique<SkeletalAnimatorImpl>(*this, skel, config))
    {
    }

    SkeletalAnimator& SkeletalAnimator::addListener(ISkeletalAnimatorListener& listener) noexcept
    {
        _impl->addListener(listener);
        return *this;
    }

    bool SkeletalAnimator::removeListener(ISkeletalAnimatorListener& listener) noexcept
    {
        return _impl->removeListener(listener);
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

    const SkeletalAnimator::Config& SkeletalAnimator::getConfig() const noexcept
    {
        return _impl->getConfig();
    }

    OptionalRef<const ISkeletalAnimatorState> SkeletalAnimator::getCurrentState() const noexcept
    {
        return _impl->getCurrentState();
    }

    OptionalRef<const ISkeletalAnimatorTransition> SkeletalAnimator::getCurrentTransition() const noexcept
    {
        return _impl->getCurrentTransition();
    }

    bool SkeletalAnimator::play(std::string_view name) noexcept
    {
        return _impl->play(name);
    }

    OzzSkeletonImporterImpl::OzzSkeletonImporterImpl(std::unique_ptr<IOzzRawSkeletonLoader>&& loader) noexcept
        : _loader(std::move(loader))
    {
    }

    size_t OzzSkeletonImporterImpl::getOutputs(const Input& input, const std::filesystem::path& basePath, std::vector<std::filesystem::path>& outputs) noexcept
    {
    }

    std::ofstream OzzSkeletonImporterImpl::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
    {

    }

    void OzzSkeletonImporterImpl::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {

    }

    std::string OzzSkeletonImporterImpl::getName() const noexcept
    {
        static const std::string name("skeleton");
        return name;
    }

    OzzSkeletonImporter::OzzSkeletonImporter(std::unique_ptr<IOzzRawSkeletonLoader>&& loader) noexcept
        : _impl(std::make_unique<OzzSkeletonImporterImpl>(loader))
    {
    }

    OzzSkeletonImporter::~OzzSkeletonImporter() noexcept
    {
    }

    size_t OzzSkeletonImporter::getOutputs(const Input& input, const std::filesystem::path& basePath, std::vector<std::filesystem::path>& outputs)
    {
        return _impl->getOutputs(input, basePath, outputs);
    }

    std::ofstream OzzSkeletonImporter::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
    {
        return _impl->createOutputStream(input, outputIndex, path);
    }

    void OzzSkeletonImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        return _impl->writeOutput(input, outputIndex, out);
    }

    std::string OzzSkeletonImporter::getName() const noexcept
    {
        return _impl->getName();
    }
}