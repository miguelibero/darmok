#pragma once

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

	class SkeletalAnimationControllerImpl final
	{
	public:
		SkeletalAnimationControllerImpl(const std::shared_ptr<Skeleton>& skeleton, const std::vector<std::shared_ptr<SkeletalAnimation>>& animations = {}) noexcept;
		void addAnimation(const std::shared_ptr<SkeletalAnimation> &anim) noexcept;
		bool playAnimation(std::string_view name, bool loop = true);
		bool update(float deltaTime) noexcept;
		void setTimeRatio(float ratio) noexcept;
		void setPlaybackSpeed(float speed) noexcept;
		glm::mat4 getModelMatrix(const std::string& joint) const noexcept;
		std::vector<glm::mat4> getBoneMatrixes(const glm::vec3& dir = {1, 0, 0}) const noexcept;
	private:
		std::shared_ptr<Skeleton> _skeleton;
		std::vector<std::shared_ptr<SkeletalAnimation>> _animations;
		std::shared_ptr<SkeletalAnimation> _currentAnimation;

		ozz::animation::SamplingJob::Context _sampling;
		ozz::vector<ozz::math::SoaTransform> _locals;
		ozz::vector<ozz::math::Float4x4> _models;

		float _timeRatio;
		float _playbackSpeed;
		bool _play;
		bool _loop;
	};
}