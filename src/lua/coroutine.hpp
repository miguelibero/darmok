#pragma once

#include <bx/bx.h>
#include <vector>
#include <memory>
#include <sol/sol.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <darmok/easing_fwd.hpp>
#include <darmok/skeleton.hpp>

namespace darmok
{
    class BX_NO_VTABLE ILuaYieldInstruction
	{
	public:
		virtual ~ILuaYieldInstruction() = default;
		virtual void update(float deltaTime) {};
		virtual bool finished() const = 0;
	};

	class LuaCombinedYieldInstruction final : public ILuaYieldInstruction
	{
	public:
		LuaCombinedYieldInstruction(const std::vector<std::shared_ptr<ILuaYieldInstruction>>& instructions) noexcept;
		void update(float deltaTime) override;
		bool finished() const noexcept override;
	private:
		std::vector<std::shared_ptr<ILuaYieldInstruction>> _instructions;
	};

	class LuaWaitForSeconds final : public ILuaYieldInstruction
	{
	public:
		LuaWaitForSeconds(float secs) noexcept;
		void update(float deltaTime) noexcept override;
		bool finished() const noexcept override;

		static void bind(sol::state_view& lua) noexcept;
	private:
		float _secs;
	};

	class Transform;

	class LuaMoveTowards final : public ILuaYieldInstruction
	{
	public:
		LuaMoveTowards(Transform& trans, const glm::vec3& position, float speed = 1.F) noexcept;
		void update(float deltaTime) noexcept override;
		bool finished() const noexcept override;

		static void bind(sol::state_view& lua) noexcept;
	private:
		std::reference_wrapper<Transform> _trans;
		glm::vec3 _position;
		float _speed;
	};

	class LuaRotateTowards final : public ILuaYieldInstruction
	{
	public:
		LuaRotateTowards(Transform& trans, const glm::quat& rotation, float speed = 1.F) noexcept;
		void update(float deltaTime) noexcept override;
		bool finished() const noexcept override;

		static void bind(sol::state_view& lua) noexcept;

	private:
		std::reference_wrapper<Transform> _trans;
		glm::quat _rotation;
		float _speed;
	};

	class LuaEasePosition final : public ILuaYieldInstruction
	{
	public:
		LuaEasePosition(Transform& trans, const glm::vec3& position, float duration = 1.F, EasingType easing = EasingType::Linear) noexcept;
		void update(float deltaTime) noexcept override;
		bool finished() const noexcept override;

		static void bind(sol::state_view& lua) noexcept;
	private:
		std::reference_wrapper<Transform> _trans;
		glm::vec3 _startPosition;
		glm::vec3 _endPosition;
		float _duration;
		float _normalizedTime;
		EasingType _easing;
	};

	class LuaEaseRotation final : public ILuaYieldInstruction
	{
	public:
		LuaEaseRotation(Transform& trans, const glm::quat& rotation, float duration = 1.F, EasingType easing = EasingType::Linear) noexcept;
		void update(float deltaTime) noexcept override;
		bool finished() const noexcept override;

		static void bind(sol::state_view& lua) noexcept;
	private:
		std::reference_wrapper<Transform> _trans;
		glm::quat _startRotation;
		glm::quat _endRotation;
		float _duration;
		float _normalizedTime;
		EasingType _easing;
	};

	class SkeletalAnimator;

	class LuaPlayAnimation final : public ILuaYieldInstruction, ISkeletalAnimatorListener
	{
	public:
		LuaPlayAnimation(SkeletalAnimator& animator, const std::string& name) noexcept;
		~LuaPlayAnimation() noexcept;
		bool finished() const noexcept override;

		static void bind(sol::state_view& lua) noexcept;
	private:
		OptionalRef<SkeletalAnimator> _animator;
		std::string _name;
		bool _finished;
		void onAnimatorDestroyed(SkeletalAnimator& animator) noexcept override;
		void onAnimatorStateFinished(SkeletalAnimator& animator, std::string_view state) noexcept override;
	};

	class LuaCoroutineThread final : public ILuaYieldInstruction
	{
	public:
		LuaCoroutineThread(const sol::thread& thread) noexcept;
		bool finished() const noexcept override;
		const sol::thread& getReal() const noexcept;
	private:
		sol::thread _thread;
	};

    class LuaCoroutineRunner final
    {
    public:
        LuaCoroutineThread startCoroutine(const sol::function& func, sol::this_state ts) noexcept;
		bool stopCoroutine(const LuaCoroutineThread& thread) noexcept;
        void update(float deltaTime, sol::state_view& lua) noexcept;

		static void bind(sol::state_view& lua) noexcept;
    private:
		std::vector<sol::thread> _threads;
		std::unordered_map<const void*, std::shared_ptr<ILuaYieldInstruction>> _awaits;

		bool doStopCoroutine(const sol::thread& thread) noexcept;
		static std::shared_ptr<ILuaYieldInstruction> readYieldInstruction(const sol::object& obj) noexcept;
    };
}