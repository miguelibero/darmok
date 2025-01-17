#pragma once

#include <memory>
#include <vector>
#include <darmok/skeleton.hpp>
#include <darmok/data.hpp>
#include <darmok/collection.hpp>
#include <ozz/base/io/stream.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/base/maths/vec_float.h>
#include <ozz/base/containers/vector.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/animation/runtime/blending_job.h>

namespace darmok
{
	class SkeletonImpl final
	{
	public:
		SkeletonImpl(ozz::animation::Skeleton&& skel) noexcept;
		ozz::animation::Skeleton& getOzz() noexcept;
		const ozz::animation::Skeleton& getOzz() const noexcept;
	private:
		ozz::animation::Skeleton _skel;
	};

	class SkeletalAnimationImpl final
	{
	public:
		SkeletalAnimationImpl(ozz::animation::Animation&& anim) noexcept;
		ozz::animation::Animation& getOzz() noexcept;
		const ozz::animation::Animation& getOzz() const noexcept;
		float getDuration() const noexcept;
	private:
		ozz::animation::Animation _anim;
	};

	class DataOzzStream : public ozz::io::Stream
	{
	public:
		DataOzzStream(const DataView& data) noexcept;
		bool opened() const noexcept override;
		size_t Read(void* buffer, size_t size) noexcept override;
		size_t Write(const void* buffer, size_t size) noexcept override;
		int Seek(int offset, Origin origin) noexcept override;
		int Tell() const noexcept override;
		size_t Size() const noexcept override;
	private:
		size_t _pos;
		DataView _data;
	};


	class BX_NO_VTABLE ISkeletalAnimationProvider
	{
	public:
		virtual ~ISkeletalAnimationProvider() = default;
		virtual std::shared_ptr<SkeletalAnimation> getAnimation(std::string_view name) = 0;
	};

	class OzzSkeletalAnimatorAnimationState final
	{
	public:
		using Definition = SkeletalAnimatorAnimationDefinition;
		OzzSkeletalAnimatorAnimationState(const ozz::animation::Skeleton& skel, const Definition& def, ISkeletalAnimationProvider& animations);
		OzzSkeletalAnimatorAnimationState(OzzSkeletalAnimatorAnimationState&& other) noexcept;

		void update(float deltaTime);
		bool hasLooped() const noexcept;
		bool hasFinished() const noexcept;
		float getDuration() const noexcept;
		float getNormalizedTime() const noexcept;
		void setNormalizedTime(float normalizedTime) noexcept;
		const ozz::vector<ozz::math::SoaTransform>& getLocals() const noexcept;
		const glm::vec2& getBlendPosition() const noexcept;
	private:
		Definition _def;
		std::shared_ptr<SkeletalAnimation> _animation;
		float _normalizedTime;
		bool _looped;
		ozz::animation::SamplingJob::Context _sampling;
		ozz::vector<ozz::math::SoaTransform> _locals;

		ozz::animation::Animation& getOzz() const noexcept;
	};

	class OzzSkeletalAnimatorState final : public ISkeletalAnimatorState
	{
	public:
		OzzSkeletalAnimatorState(const ozz::animation::Skeleton& skel, const Definition& def, ISkeletalAnimationProvider& animations) noexcept;

		void update(float deltaTime, const glm::vec2& blendPosition);
		std::string_view getName() const noexcept override;
		const ozz::vector<ozz::math::SoaTransform>& getLocals() const;
		float getNormalizedTime() const noexcept override;
		float getDuration() const noexcept override;
		bool hasLooped() const noexcept;
		bool hasFinished() const noexcept;
		const std::string& getNextState() const noexcept;
		void setSpeed(float speed) noexcept;
		void afterFinished() noexcept;
	private:
		glm::vec2 getBlendedPosition(float f) const noexcept;
		float calcDuration() const noexcept;

		using AnimationState = OzzSkeletalAnimatorAnimationState;
		Definition _def;
		std::vector<AnimationState> _animations;
		ozz::vector<ozz::math::SoaTransform> _locals;
		ozz::vector<ozz::animation::BlendingJob::Layer> _layers;
		glm::vec2 _blendPos;
		glm::vec2 _oldBlendPos;
		float _normalizedTweenTime;
		float _normalizedTime;
		bool _looped;
		float _duration;
		float _speed;
	};

	class OzzSkeletalAnimatorTransition final : public ISkeletalAnimatorTransition
	{
	public:
		using State = OzzSkeletalAnimatorState;

		OzzSkeletalAnimatorTransition(const Definition& def, State&& currentState, State&& previousState) noexcept;
		void update(float deltaTime, const glm::vec2& blendPosition);
		float getDuration() const noexcept override;
		float getNormalizedTime() const noexcept override;
		void setNormalizedTime(float normalizedTime) noexcept;
		bool hasFinished() const noexcept;
		State&& finish() noexcept;

		const ozz::vector<ozz::math::SoaTransform>& getLocals() const noexcept;

		const State& getCurrentOzzState() const noexcept;
		const State& getPreviousOzzState() const noexcept;
		State& getCurrentOzzState() noexcept;
		State& getPreviousOzzState() noexcept;

		const ISkeletalAnimatorState& getCurrentState() const noexcept override;
		const ISkeletalAnimatorState& getPreviousState() const noexcept override;
		ISkeletalAnimatorState& getCurrentState() noexcept;
		ISkeletalAnimatorState& getPreviousState() noexcept;
	private:
		Definition _def;
		State _currentState;
		State _previousState;
		ozz::vector<ozz::math::SoaTransform> _locals;
		float _normalizedTime;
	};

	class SkeletalAnimatorImpl final : public ISkeletalAnimationProvider
	{
	public:
		using Definition = SkeletalAnimatorDefinition;
		using Transition = OzzSkeletalAnimatorTransition;
		using State = OzzSkeletalAnimatorState;
		using AnimationMap = SkeletalAnimationMap;
		using PlaybackState = SkeletalAnimatorPlaybackState;

		SkeletalAnimatorImpl(SkeletalAnimator& animator, const std::shared_ptr<Skeleton>& skeleton, const AnimationMap& animations, const Definition& def) noexcept;
		~SkeletalAnimatorImpl();

		void addListener(std::unique_ptr<ISkeletalAnimatorListener>&& listener) noexcept;
		void addListener(ISkeletalAnimatorListener& listener) noexcept;
		bool removeListener(const ISkeletalAnimatorListener& listener) noexcept;
		size_t removeListeners(const ISkeletalAnimatorListenerFilter& filter) noexcept;

		void setPlaybackSpeed(float speed) noexcept;
		float getPlaybackSpeed() const noexcept;

		void setBlendPosition(const glm::vec2& value) noexcept;
		const glm::vec2& getBlendPosition() const noexcept;

		const Definition& getDefinition() const noexcept;
		OptionalRef<const ISkeletalAnimatorState> getCurrentState() const noexcept;
		OptionalRef<const ISkeletalAnimatorTransition> getCurrentTransition() const noexcept;
		float getStateDuration(const std::string& name) const noexcept;

		bool play(std::string_view name, float stateSpeed = 1.F) noexcept;
		void stop() noexcept;
		void pause() noexcept;
		PlaybackState getPlaybackState() noexcept;

		void update(float deltaTime);

		glm::mat4 getJointModelMatrix(const std::string& joint) const noexcept;
		std::unordered_map<std::string, glm::mat4> getBoneModelMatrixes(const glm::vec3& dir = {1, 0, 0}) const noexcept;

		// ISkeletalAnimationProvider
		std::shared_ptr<SkeletalAnimation> getAnimation(std::string_view name) noexcept override;
	private:
		using TransitionKey = std::pair<std::string, std::string>;
		AnimationMap _animations;
		SkeletalAnimator& _animator;
		std::shared_ptr<Skeleton> _skeleton;
		Definition _def;
		float _speed;
		bool _paused;
		glm::vec2 _blendPosition;

		std::optional<Transition> _transition;
		std::optional<State> _state;
		ozz::vector<ozz::math::Float4x4> _models;

		OwnRefCollection<ISkeletalAnimatorListener> _listeners;

		ozz::animation::Skeleton& getOzz() noexcept;
		const ozz::animation::Skeleton& getOzz() const noexcept;
		void afterUpdate() noexcept;

		OptionalRef<State> getCurrentOzzState() noexcept;
		OptionalRef<Transition> getCurrentOzzTransition() noexcept;
	};
}