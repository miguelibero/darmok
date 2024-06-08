#pragma once

#include <unordered_set>
#include <darmok/skeleton.hpp>
#include <darmok/data.hpp>
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

	class IDataLoader;

    class OzzSkeletonLoader final : public ISkeletonLoader
	{
	public:
		OzzSkeletonLoader(IDataLoader& dataLoader) noexcept;
		std::shared_ptr<Skeleton> operator()(std::string_view name) override;
	private:
		IDataLoader& _dataLoader;
	};

	class OzzSkeletalAnimationLoader final : public ISkeletalAnimationLoader
	{
	public:
		OzzSkeletalAnimationLoader(IDataLoader& dataLoader) noexcept;
		std::shared_ptr<SkeletalAnimation> operator()(std::string_view name) override;
	private:
		IDataLoader& _dataLoader;
	};

	class OzzSkeletalAnimatorAnimationState final
	{
	public:
		using Config = SkeletalAnimatorAnimationConfig;
		OzzSkeletalAnimatorAnimationState(const ozz::animation::Skeleton& skel, const Config& config) noexcept;
		OzzSkeletalAnimatorAnimationState(OzzSkeletalAnimatorAnimationState&& other) noexcept;

		void update(float deltaTime);
		bool hasLoopedDuringLastUpdate() const noexcept;
		float getDuration() const noexcept;
		float getNormalizedTime() const noexcept;
		void setNormalizedTime(float normalizedTime) noexcept;
		const ozz::vector<ozz::math::SoaTransform>& getLocals() const noexcept;
		const glm::vec2& getBlendPosition() const noexcept;
	private:
		Config _config;
		float _normalizedTime;
		bool _lastUpdateLooped;
		ozz::animation::SamplingJob::Context _sampling;
		ozz::vector<ozz::math::SoaTransform> _locals;

		ozz::animation::Animation& getOzz() const noexcept;
	};

	class OzzSkeletalAnimatorState final : public ISkeletalAnimatorState
	{
	public:
		OzzSkeletalAnimatorState(const ozz::animation::Skeleton& skel, const Config& config) noexcept;

		void update(float deltaTime, const glm::vec2& blendPosition);
		std::string_view getName() const noexcept override;
		const ozz::vector<ozz::math::SoaTransform>& getLocals() const;
	private:
		using AnimationState = OzzSkeletalAnimatorAnimationState;
		Config _config;
		std::vector<AnimationState> _animations;
		ozz::vector<ozz::math::SoaTransform> _locals;
		ozz::vector<ozz::animation::BlendingJob::Layer> _layers;
	};

	class OzzSkeletalAnimatorTransition final : public ISkeletalAnimatorTransition
	{
	public:
		using State = OzzSkeletalAnimatorState;

		OzzSkeletalAnimatorTransition(const Config& config, State&& currentState, State&& previousState) noexcept;
		void update(float deltaTime, const glm::vec2& blendPosition);
		float getDuration() const noexcept override;
		float getNormalizedTime() const noexcept override;
		void setNormalizedTime(float normalizedTime) noexcept;
		bool hasFinished() const noexcept;
		State&& finish() noexcept;

		const ozz::vector<ozz::math::SoaTransform>& getLocals() const noexcept;

		State& getCurrentOzzState() noexcept;
		State& getPreviousOzzState() noexcept;

		const ISkeletalAnimatorState& getCurrentState() const noexcept override;
		const ISkeletalAnimatorState& getPreviousState() const noexcept override;
	private:
		Config _config;
		State _currentState;
		State _previousState;
		ozz::vector<ozz::math::SoaTransform> _locals;
		float _normalizedTime;
	};

	class SkeletalAnimatorImpl final
	{
	public:
		using Config = SkeletalAnimatorConfig;
		using Transition = OzzSkeletalAnimatorTransition;
		using State = OzzSkeletalAnimatorState;

		SkeletalAnimatorImpl(SkeletalAnimator& animator, const std::shared_ptr<Skeleton>& skeleton, const Config& config) noexcept;

		void addListener(ISkeletalAnimatorListener& listener) noexcept;
		bool removeListener(ISkeletalAnimatorListener& listener) noexcept;

		void setPlaybackSpeed(float speed) noexcept;
		float getPlaybackSpeed() const noexcept;
		void setBlendPosition(const glm::vec2& value) noexcept;

		const Config& getConfig() const noexcept;
		OptionalRef<const ISkeletalAnimatorState> getCurrentState() const noexcept;
		OptionalRef<const ISkeletalAnimatorTransition> getCurrentTransition() const noexcept;

		bool play(std::string_view name) noexcept;
		void update(float deltaTime);

		glm::mat4 getJointModelMatrix(const std::string& joint) const noexcept;
		std::vector<glm::mat4> getBoneModelMatrixes(const glm::vec3& dir = {1, 0, 0}) const noexcept;
	private:
		using TransitionKey = std::pair<std::string, std::string>;

		SkeletalAnimator& _animator;
		std::shared_ptr<Skeleton> _skeleton;
		Config _config;
		float _speed;
		glm::vec2 _blendPosition;

		std::optional<Transition> _transition;
		std::optional<State> _state;
		ozz::vector<ozz::math::Float4x4> _models;

		std::unordered_set<OptionalRef<ISkeletalAnimatorListener>> _listeners;

		ozz::animation::Skeleton& getOzz() noexcept;
		const ozz::animation::Skeleton& getOzz() const noexcept;
		void afterUpdate() noexcept;
	};
}