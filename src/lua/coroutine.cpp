#include "lua/coroutine.hpp"
#include "lua/utils.hpp"
#include "lua/glm.hpp"
#include <darmok/transform.hpp>
#include <darmok/math.hpp>
#include <darmok/easing.hpp>
#include <darmok/skeleton.hpp>

namespace darmok
{
	LuaCoroutine::LuaCoroutine(const LuaCoroutineRunner& runner, const void* coroutinePtr) noexcept
		: _runner{ runner }
		, _coroutinePtr{ coroutinePtr }
	{
	}

	bool LuaCoroutine::finished() const noexcept
	{
		return _runner.get().hasFinished(_coroutinePtr);
	}

	const void* LuaCoroutine::getPointer() const noexcept
	{
		return _coroutinePtr;
	}

	LuaCombinedYieldInstruction::LuaCombinedYieldInstruction(const std::vector<std::shared_ptr<ILuaYieldInstruction>>& instructions) noexcept
		: _instructions{ instructions }
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
		: _secs{ secs }
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
		: _trans{ trans }
		, _position{ position }
		, _speed{ speed }
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
		: _trans{ trans }
		, _rotation{ rotation }
		, _speed{ speed }
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
		: _trans{ trans }
		, _startPosition{ trans.getPosition() }
		, _endPosition{ position }
		, _duration{ duration }
		, _normalizedTime{ 0.F }
		, _easing{ easing }
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
		: _trans{ trans }
		, _startRotation{ trans.getRotation() }
		, _endRotation{ rotation }
		, _angle{ 0.F }
		, _axis{ 0 }
		, _duration{ duration }
		, _normalizedTime{ 0.F }
		, _easing{ easing }
	{
	}

	LuaEaseRotation::LuaEaseRotation(Transform& trans, float angle, const glm::vec3& axis, float duration, EasingType easing) noexcept
		: _trans{ trans }
		, _startRotation{ trans.getRotation() }
		, _endRotation{ _startRotation * glm::angleAxis(angle, axis) }
		, _angle{ angle }
		, _axis{ axis }
		, _duration{ duration }
		, _normalizedTime{ 0.F }
		, _easing{ easing }
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
		: _trans{ trans }
		, _startScale{ trans.getScale() }
		, _endScale{ scale }
		, _duration{ duration }
		, _normalizedTime{ 0.F }
		, _easing{ easing }
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
		: _animator{ animator }
		, _name{ name }
		, _finished{ false }
	{
		_animator->addListener(*this);
		auto state = _animator->getCurrentState();
		if (!state || state->getName() != _name)
		{
			if (!_animator->play(_name))
			{
				_finished = true;
			}
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

	LuaCoroutineRunner::Coroutines::const_iterator LuaCoroutineRunner::findCoroutine(const void* coroutinePtr) const noexcept
	{
		return std::find_if(_coroutines.begin(), _coroutines.end(), [coroutinePtr](auto& bundle) {
			return bundle->coroutine.pointer() == coroutinePtr;
		});
	}

	bool LuaCoroutineRunner::hasFinished(const void* coroutinePtr) const noexcept
	{
		return findCoroutine(coroutinePtr) == _coroutines.end();
	}

	// https://github.com/ThePhD/sol2/issues/296
	LuaBundledCoroutine::LuaBundledCoroutine(const sol::main_function& func) noexcept
		: thread{ sol::thread::create(func.lua_state()) }
	{
		// thread needs to be defined after coroutine
		// to guarantee that it's destroyed afterwards
		coroutine = sol::coroutine(thread.state(), func);
	}

    LuaCoroutine LuaCoroutineRunner::startCoroutine(const sol::function& func) noexcept
	{
		auto bundle = std::make_shared<LuaBundledCoroutine>(func);
		auto coroutinePtr = bundle->coroutine.pointer();
		_coroutines.push_back(std::move(bundle));
		return LuaCoroutine(*this, coroutinePtr);
	}

	bool LuaCoroutineRunner::stopCoroutine(const LuaCoroutine& coroutine) noexcept
	{
		return doStopCoroutine(coroutine.getPointer());
	}

	bool LuaCoroutineRunner::doStopCoroutine(const void* coroutinePtr) noexcept
	{
		auto itr = findCoroutine(coroutinePtr);
		auto found = itr != _coroutines.end();
		if (found)
		{
			_coroutines.erase(itr);
		}
		auto itr2 = _awaits.find(coroutinePtr);
		if (itr2 != _awaits.end())
		{
			_awaits.erase(itr2);
		}
		return found;
	}

	expected<void, std::string> LuaCoroutineRunner::update(float deltaTime) noexcept
	{
		std::vector<const void*> finishedCoroutines;
		for (auto& bundle : Coroutines{ _coroutines })
		{
			auto& coroutine = bundle->coroutine;
			auto coroutinePtr = coroutine.pointer();
			auto itr = _awaits.find(coroutinePtr);
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
			auto resumeResult = resumeCoroutine(coroutine);
			if (!resumeResult)
			{
				return unexpected{ std::move(resumeResult).error() };
			}
			if (!resumeResult.value())
			{
				finishedCoroutines.push_back(coroutinePtr);
			}
		}
		for (auto coroutinePtr : finishedCoroutines)
		{
			doStopCoroutine(coroutinePtr);
		}

		return {};
	}

	expected<bool, std::string> LuaCoroutineRunner::resumeCoroutine(sol::coroutine& coroutine) noexcept
	{
		if (!coroutine.runnable())
		{
			return false;
		}
		auto result = LuaUtils::wrapResult(coroutine(), "running coroutine");
		if(!result)
		{
			return unexpected{ std::move(result).error() };
		}
		auto robj = result.value();
		if (auto instr = readYieldInstruction(robj))
		{
			_awaits.emplace(coroutine.pointer(), std::move(instr));
			return true;
		}
		return false;
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
		if (obj.is<LuaCoroutine>())
		{
			return std::make_shared<LuaCoroutine>(obj.as<LuaCoroutine>());
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

		LuaUtils::newEnum<EasingType>(lua, "EasingType");
	}

}