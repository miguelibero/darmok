#include "coroutine.hpp"
#include "utils.hpp"
#include "glm.hpp"
#include <darmok/transform.hpp>
#include <darmok/math.hpp>
#include <darmok/easing.hpp>
#include <darmok/skeleton.hpp>

namespace darmok
{
    LuaCoroutineThread::LuaCoroutineThread(const sol::thread& thread) noexcept
		: _thread(thread)
	{
	}

	bool LuaCoroutineThread::finished() const noexcept
	{
		return _thread.status() == sol::thread_status::dead;
	}

	const sol::thread& LuaCoroutineThread::getReal() const noexcept
	{
		return _thread;
	}

	LuaCombinedYieldInstruction::LuaCombinedYieldInstruction(const std::vector<std::shared_ptr<ILuaYieldInstruction>>& instructions) noexcept
		: _instructions(instructions)
	{
	}

	void LuaCombinedYieldInstruction::update(float deltaTime)
	{
		for (auto& instr : _instructions)
		{
			instr->update(deltaTime);
		}
	}

	bool LuaCombinedYieldInstruction::finished() const noexcept
	{
		for (auto& instr : _instructions)
		{
			if (!instr->finished())
			{
				return false;
			}
		}
		return true;
	}

	LuaWaitForSeconds::LuaWaitForSeconds(float secs) noexcept
		: _secs(secs)
	{
	}

	void LuaWaitForSeconds::update(float deltaTime) noexcept
	{
		_secs -= deltaTime;
	}

	bool LuaWaitForSeconds::finished() const noexcept
	{
		return _secs <= 0;
	}

	void LuaWaitForSeconds::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaWaitForSeconds>("WaitForSeconds",
			sol::factories(
				[](float secs) { return std::make_shared<LuaWaitForSeconds>(secs); }
			), sol::base_classes, sol::bases<ILuaYieldInstruction>()
		);
	}

	LuaMoveTowards::LuaMoveTowards(Transform& trans, const glm::vec3& position, float speed) noexcept
		: _trans(trans)
		, _position(position)
		, _speed(speed)
	{
	}

	void LuaMoveTowards::update(float deltaTime) noexcept
	{
		glm::vec3 pos = _trans.get().getPosition();
		pos = Math::moveTowards(pos, _position, deltaTime * _speed);
		_trans.get().setPosition(pos);
	}

	bool LuaMoveTowards::finished() const noexcept
	{
		return _trans.get().getPosition() == _position;
	}


	void LuaMoveTowards::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaMoveTowards>("MoveTowards",
			sol::factories(
				[](Transform& trans, const VarLuaTable<glm::vec3>& pos, float speed) -> std::shared_ptr<ILuaYieldInstruction> {
					return std::make_shared<LuaMoveTowards>(trans, LuaGlm::tableGet(pos), speed);
				}
			), sol::base_classes, sol::bases<ILuaYieldInstruction>()
		);
	}

	LuaRotateTowards::LuaRotateTowards(Transform& trans, const glm::quat& rotation, float speed) noexcept
		: _trans(trans)
		, _rotation(rotation)
		, _speed(speed)
	{
	}

	void LuaRotateTowards::update(float deltaTime) noexcept
	{
		glm::quat rot = _trans.get().getRotation();
		rot = Math::rotateTowards(rot, _rotation, deltaTime * _speed);
		_trans.get().setRotation(rot);
	}

	bool LuaRotateTowards::finished() const noexcept
	{
		return _trans.get().getRotation() == _rotation;
	}

	void LuaRotateTowards::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaRotateTowards>("RotateTowards",
			sol::factories(
				[](Transform& trans, const VarLuaTable<glm::quat>& rot, float speed) -> std::shared_ptr<ILuaYieldInstruction> {
					return std::make_shared<LuaRotateTowards>(trans, LuaGlm::tableGet(rot), speed);
				}
			), sol::base_classes, sol::bases<ILuaYieldInstruction>()
		);
	}

	LuaEasePosition::LuaEasePosition(Transform& trans, const glm::vec3& position, float duration, EasingType easing) noexcept
		: _trans(trans)
		, _startPosition(trans.getPosition())
		, _endPosition(position)
		, _duration(duration)
		, _normalizedTime(0.F)
		, _easing(easing)
	{
	}

	void LuaEasePosition::update(float deltaTime) noexcept
	{
		_normalizedTime += deltaTime / _duration;
		if (_normalizedTime > 1.F)
		{
			_trans.get().setPosition(_endPosition);
			_normalizedTime = 1.F;
			return;
		}
		auto f = Easing::apply(_easing, _normalizedTime, 0.F, 1.F);
		auto pos = Math::lerp(_startPosition, _endPosition, f);
		_trans.get().setPosition(pos);
	}

	bool LuaEasePosition::finished() const noexcept
	{
		return _trans.get().getPosition() == _endPosition;
	}

	void LuaEasePosition::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaEasePosition>("EasePosition",
			sol::factories(
				[](Transform& trans, const VarLuaTable<glm::vec3>& pos, float duration, EasingType easing) -> std::shared_ptr<ILuaYieldInstruction> {
					return std::make_shared<LuaEasePosition>(trans, LuaGlm::tableGet(pos), duration, easing);
				},
				[](Transform& trans, const VarLuaTable<glm::vec3>& pos, float duration) -> std::shared_ptr<ILuaYieldInstruction> {
					return std::make_shared<LuaEasePosition>(trans, LuaGlm::tableGet(pos), duration);
				}
			), sol::base_classes, sol::bases<ILuaYieldInstruction>()
		);
	}

	LuaEaseRotation::LuaEaseRotation(Transform& trans, const glm::quat& rotation, float duration, EasingType easing) noexcept
		: _trans(trans)
		, _startRotation(trans.getRotation())
		, _endRotation(rotation)
		, _angle(0.F)
		, _axis(0)
		, _duration(duration)
		, _normalizedTime(0.F)
		, _easing(easing)
	{
	}

	LuaEaseRotation::LuaEaseRotation(Transform& trans, float angle, const glm::vec3& axis, float duration, EasingType easing) noexcept
		: _trans(trans)
		, _startRotation(trans.getRotation())
		, _endRotation(_startRotation * glm::angleAxis(angle, axis))
		, _angle(angle)
		, _axis(axis)
		, _duration(duration)
		, _normalizedTime(0.F)
		, _easing(easing)
	{
	}

	void LuaEaseRotation::update(float deltaTime) noexcept
	{
		_normalizedTime += deltaTime / _duration;
		if (_normalizedTime > 1.F)
		{
			_trans.get().setRotation(_endRotation);
			_normalizedTime = 1.F;
			return;
		}
		auto f = Easing::apply(_easing, _normalizedTime, 0.F, 1.F);

		glm::quat rot(1.F, 0.F, 0.F, 0.F);
		if (_angle != 0.F)
		{
			rot = _startRotation * glm::angleAxis(_angle * _normalizedTime, _axis);
		}
		else
		{
			rot = glm::slerp(_startRotation, _endRotation, f);
		}

		_trans.get().setRotation(rot);
	}

	bool LuaEaseRotation::finished() const noexcept
	{
		return _trans.get().getRotation() == _endRotation;
	}

	void LuaEaseRotation::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaEaseRotation>("EaseRotation",
			sol::factories(
				[](Transform& trans, const VarLuaTable<glm::quat>& rot, float duration, EasingType easing) -> std::shared_ptr<ILuaYieldInstruction> {
					return std::make_shared<LuaEaseRotation>(trans, LuaGlm::tableGet(rot), duration, easing);
				},
				[](Transform& trans, const VarLuaTable<glm::quat>& rot, float duration) -> std::shared_ptr<ILuaYieldInstruction> {
					return std::make_shared<LuaEaseRotation>(trans, LuaGlm::tableGet(rot), duration);
				},
				[](Transform& trans, float angle, const VarLuaTable<glm::vec3>& axis, float duration, EasingType easing) -> std::shared_ptr<ILuaYieldInstruction> {
					return std::make_shared<LuaEaseRotation>(trans, angle, LuaGlm::tableGet(axis), duration, easing);
				},
				[](Transform& trans, float angle, const VarLuaTable<glm::vec3>& axis, float duration) -> std::shared_ptr<ILuaYieldInstruction> {
					return std::make_shared<LuaEaseRotation>(trans, angle, LuaGlm::tableGet(axis), duration);
				}
			), sol::base_classes, sol::bases<ILuaYieldInstruction>()
		);
	}

	LuaEaseScale::LuaEaseScale(Transform& trans, const glm::vec3& scale, float duration, EasingType easing) noexcept
		: _trans(trans)
		, _startScale(trans.getScale())
		, _endScale(scale)
		, _duration(duration)
		, _normalizedTime(0.F)
		, _easing(easing)
	{
	}

	void LuaEaseScale::update(float deltaTime) noexcept
	{
		_normalizedTime += deltaTime / _duration;
		if (_normalizedTime > 1.F)
		{
			_trans.get().setScale(_endScale);
			_normalizedTime = 1.F;
			return;
		}
		auto f = Easing::apply(_easing, _normalizedTime, 0.F, 1.F);
		auto scale = Math::lerp(_startScale, _endScale, f);
		_trans.get().setScale(scale);
	}

	bool LuaEaseScale::finished() const noexcept
	{
		return _trans.get().getScale() == _endScale;
	}

	void LuaEaseScale::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaEaseScale>("EaseScale",
			sol::factories(
				[](Transform& trans, const VarLuaVecTable<glm::vec3>& scale, float duration, EasingType easing) -> std::shared_ptr<ILuaYieldInstruction> {
					return std::make_shared<LuaEaseScale>(trans, LuaGlm::tableGet(scale), duration, easing);
				},
				[](Transform& trans, const VarLuaVecTable<glm::vec3>& scale, float duration) -> std::shared_ptr<ILuaYieldInstruction> {
					return std::make_shared<LuaEaseScale>(trans, LuaGlm::tableGet(scale), duration);
				}
			), sol::base_classes, sol::bases<ILuaYieldInstruction>()
		);
	}

	LuaPlayAnimation::LuaPlayAnimation(SkeletalAnimator& animator, const std::string& name) noexcept
		: _animator(animator)
		, _name(name)
		, _finished(false)
	{
		_animator->addListener(*this);
		auto state = _animator->getCurrentState();
		if (!state || state->getName() != _name)
		{
			_animator->play(_name);
		}
	}

	LuaPlayAnimation::~LuaPlayAnimation() noexcept
	{
		if (_animator)
		{
			_animator->removeListener(*this);
		}
	}

	bool LuaPlayAnimation::finished() const noexcept
	{
		return _finished;
	}

	void LuaPlayAnimation::onAnimatorDestroyed(SkeletalAnimator& animator) noexcept
	{
		_animator.reset();
	}

	void LuaPlayAnimation::onAnimatorStateFinished(SkeletalAnimator& animator, std::string_view state) noexcept
	{
		if (state != _name)
		{
			return;
		}
		_finished = true;
		if (_animator)
		{
			_animator->removeListener(*this);
		}
		_animator.reset();
	}

	void LuaPlayAnimation::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaPlayAnimation>("PlayAnimation",
			sol::factories(
				[](SkeletalAnimator& anim, const std::string& name) -> std::shared_ptr<ILuaYieldInstruction> {
					return std::make_shared<LuaPlayAnimation>(anim, name);
				}
			), sol::base_classes, sol::bases<ILuaYieldInstruction>()
		);
	}

    LuaCoroutineThread LuaCoroutineRunner::startCoroutine(const sol::function& func, sol::this_state ts) noexcept
	{
		sol::state_view lua(ts);
		sol::thread thread = lua["coroutine"]["create"](func);
		_threads.push_back(thread);
		return LuaCoroutineThread(thread);
	}

	bool LuaCoroutineRunner::stopCoroutine(const LuaCoroutineThread& thread) noexcept
	{
		return doStopCoroutine(thread.getReal());
	}

	bool LuaCoroutineRunner::doStopCoroutine(const sol::thread& thread) noexcept
	{
		auto itr = std::remove(_threads.begin(), _threads.end(), thread);
		auto found = itr != _threads.end();
		if (found)
		{
			_threads.erase(itr, _threads.end());
		}
		auto itr2 = _awaits.find(thread.pointer());
		if (itr2 != _awaits.end())
		{
			_awaits.erase(itr2);
		}
		return found;
	}

    void LuaCoroutineRunner::update(float deltaTime, sol::state_view& lua) noexcept
	{
		std::vector<sol::thread> finished;
		std::vector<sol::thread> threads(_threads);
		sol::protected_function resume = lua["coroutine"]["resume"];
		for (auto& thread : threads)
		{
			auto itr = _awaits.find(thread.pointer());
			if (itr != _awaits.end())
			{
				auto& instr = *itr->second;
				instr.update(deltaTime);
				if (instr.finished())
				{
					_awaits.erase(itr);
				}
				else
				{
					continue;
				}
			}
			auto status = thread.status();
			if (status == sol::thread_status::yielded)
			{
				static const std::string logDesc = "resuming coroutine";
				auto result = resume(thread, deltaTime);
				LuaUtils::checkResult(logDesc, result);
				std::tuple<bool, sol::object> r = result;
				auto robj = std::get<sol::object>(r);
				if (!std::get<bool>(r))
				{
					LuaUtils::logError(logDesc, robj.as<sol::error>());
					status = sol::thread_status::dead;
				}
				else if (auto instr = readYieldInstruction(robj))
				{
					_awaits.emplace(thread.pointer(), std::move(instr));
				}
			}
			if (status == sol::thread_status::dead)
			{
				finished.emplace_back(thread);
				continue;
			}
		}
		for (auto& coroutine : finished)
		{
			doStopCoroutine(coroutine);
		}
	}

	std::shared_ptr<ILuaYieldInstruction> LuaCoroutineRunner::readYieldInstruction(const sol::object& obj) noexcept
	{
		auto type = obj.get_type();
		if (type == sol::type::table)
		{
			auto table = obj.as<sol::table>();
			size_t count = table.size();
			std::vector<std::shared_ptr<ILuaYieldInstruction>> instructions;
			instructions.reserve(count);
			for (size_t i = 1; i <= count; ++i)
			{
				if (auto instr = readYieldInstruction(table[i]))
				{
					instructions.push_back(instr);
				}
			}
			return std::make_shared<LuaCombinedYieldInstruction>(instructions);
		}
		if (obj.get_type() == sol::type::number)
		{
			return std::make_shared<LuaWaitForSeconds>(obj.as<float>());
		}
		if (obj.is<std::shared_ptr<ILuaYieldInstruction>>())
		{
			return obj.as<std::shared_ptr<ILuaYieldInstruction>>();
		}
		if (obj.is<LuaCombinedYieldInstruction>())
		{
			return std::make_shared<LuaCombinedYieldInstruction>(obj.as<LuaCombinedYieldInstruction>());
		}
		if (obj.is<LuaWaitForSeconds>())
		{
			return std::make_shared<LuaWaitForSeconds>(obj.as<LuaWaitForSeconds>());
		}
		if (obj.is<LuaCoroutineThread>())
		{
			return std::make_shared<LuaCoroutineThread>(obj.as<LuaCoroutineThread>());
		}
		return nullptr;
	}

	void LuaCoroutineRunner::bind(sol::state_view& lua) noexcept
	{
		LuaWaitForSeconds::bind(lua);
		LuaMoveTowards::bind(lua);
		LuaRotateTowards::bind(lua);
		LuaEasePosition::bind(lua);
		LuaEaseRotation::bind(lua);
		LuaEaseScale::bind(lua);
		LuaPlayAnimation::bind(lua);

		LuaUtils::newEnumFunc<EasingType>(lua, "EasingType", EasingType::Count, &Easing::getTypeName);
	}

}