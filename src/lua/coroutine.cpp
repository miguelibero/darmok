#include "coroutine.hpp"
#include "utils.hpp"
#include "glm.hpp"
#include <darmok/transform.hpp>
#include <darmok/math.hpp>

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
			)
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
			)
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
			)
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
				auto result = resume(thread, deltaTime);
				LuaUtils::checkResult("resuming coroutine", result);
				std::tuple<bool, sol::object> r = result;
				if (!std::get<bool>(r))
				{
					status = sol::thread_status::dead;
				}
				else if (auto instr = readYieldInstruction(std::get<sol::object>(r)))
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
	}

}