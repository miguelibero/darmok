#pragma once

#include <optional>
#include <vector>
#include <sol/sol.hpp>
#include <glm/glm.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok
{
    class Keyboard;

	class LuaKeyboard final
	{
	public:
		LuaKeyboard(Keyboard& kb) noexcept;
		bool getKey(const std::string& name) const noexcept;
		std::string getUpdateChars() const noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		OptionalRef<Keyboard> _kb;
	};

	class Mouse;

	class LuaMouse final
	{
	public:
		LuaMouse(Mouse& mouse) noexcept;
		const glm::vec2& getPosition() const noexcept;
		glm::vec2 getPositionDelta() const noexcept;
		const glm::vec2& getScroll() const noexcept;
		glm::vec2 getScrollDelta() const noexcept;
		bool getLeftButton() const noexcept;
		bool getMiddleButton() const noexcept;
		bool getRightButton() const noexcept;
		bool getActive() const noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		OptionalRef<Mouse> _mouse;
	};

	class Gamepad;

	class LuaGamepad final
	{
	public:
		LuaGamepad(Gamepad& gamepad) noexcept;
		bool getButton(const std::string& name) const noexcept;
		const glm::ivec3& getLeftStick() const noexcept;
		const glm::ivec3& getRightStick() const noexcept;
		bool isConnected() const noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		OptionalRef<Gamepad> _gamepad;
	};

	class Input;

	class LuaInput final
	{
	public:
		LuaInput(Input& input) noexcept;
		void addBindings(const std::string& name, const sol::table& data) noexcept;
		LuaKeyboard getKeyboard() noexcept;
		LuaMouse getMouse() noexcept;
		std::optional<LuaGamepad> getGamepad(uint8_t num = 0) noexcept;
		std::vector<LuaGamepad> getGamepads() noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		OptionalRef<Input> _input;
	};
}