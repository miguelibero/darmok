#pragma once

#include <unordered_set>
#include <darmok/skeleton.hpp>
#include <darmok/data.hpp>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/animation/runtime/animation.h>
#include <ozz/base/io/stream.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/base/maths/vec_float.h>
#include <ozz/base/containers/vector.h>

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

	class OzzSkeletalAnimatorState final : public ISkeletalAnimatorState
	{
	public:
		OzzSkeletalAnimatorState(const ozz::animation::Skeleton& skel, const Config& config) noexcept;
		OzzSkeletalAnimatorState(OzzSkeletalAnimatorState&& other) noexcept;

		std::string_view getName() const noexcept override;
		void update(float deltaTime);
		bool hasLoopedDuringLastUpdate() const noexcept;
		float getNormalizedTime() const noexcept override;
		float getDuration() const noexcept override;
		void setNormalizedTime(float normalizedTime) noexcept override;
		const ozz::vector<ozz::math::SoaTransform>& getLocals() const noexcept;
	private:
		Config _config;
		float _normalizedTime;
		bool _lastUpdateLooped;
		ozz::animation::SamplingJob::Context _sampling;
		ozz::vector<ozz::math::SoaTransform> _locals;

		ozz::animation::Animation& getOzz() const noexcept;
	};

	class OzzSkeletalAnimatorTransition final : public ISkeletalAnimatorTransition
	{
	public:
		using State = OzzSkeletalAnimatorState;

		OzzSkeletalAnimatorTransition(const Config& config, State&& currentState, State&& previousState) noexcept;
		void update(float deltaTime);
		float getNormalizedTime() const noexcept override;
		void setNormalizedTime(float normalizedTime) noexcept override;
		bool hasStartedDuringLastUpdate() const noexcept;
		float getDuration() const noexcept override;
		bool hasFinished() const noexcept;
		State&& finish() noexcept;

		const ozz::vector<ozz::math::SoaTransform>& getLocals() const noexcept;

		State& getCurrentOzzState() noexcept;
		State& getPreviousOzzState() noexcept;

		ISkeletalAnimatorState& getCurrentState() noexcept override;
		ISkeletalAnimatorState& getPreviousState() noexcept override;
	private:
		Config _config;
		State _currentState;
		State _previousState;
		bool _finished;
		bool _lastUpdateStarted;
		ozz::vector<ozz::math::SoaTransform> _locals;
		bool _blending;
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

		const Config& getConfig() const noexcept;
		OptionalRef<ISkeletalAnimatorState> getCurrentState() noexcept;
		OptionalRef<ISkeletalAnimatorTransition> getCurrentTransition() noexcept;

		bool play(std::string_view name) noexcept;
		void update(float deltaTime);

		glm::mat4 getModelMatrix(const std::string& joint) const noexcept;
		std::vector<glm::mat4> getBoneMatrixes(const glm::vec3& dir = {1, 0, 0}) const noexcept;
	private:
		using TransitionKey = std::pair<std::string, std::string>;

		SkeletalAnimator& _animator;
		std::shared_ptr<Skeleton> _skeleton;
		Config _config;
		float _speed;

		std::optional<Transition> _transition;
		std::optional<State> _state;
		ozz::vector<ozz::math::Float4x4> _models;

		std::unordered_set<OptionalRef<ISkeletalAnimatorListener>> _listeners;

		ozz::animation::Skeleton& getOzz() noexcept;
		const ozz::animation::Skeleton& getOzz() const noexcept;
		void afterUpdate() noexcept;
		void checkStateLooped(State& state) noexcept;
	};
}