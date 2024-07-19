#pragma once

#include <optional>
#include <vector>
#include <array>
#include <sol/sol.hpp>
#include <darmok/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/input.hpp>

namespace darmok
{
    class Keyboard;

	class LuaKeyboard final
	{
	public:
		LuaKeyboard(Keyboard& kb) noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		std::reference_wrapper<Keyboard> _kb;

		bool getKey(const sol::variadic_args& names) const noexcept;
		std::string getUpdateChars() const noexcept;
		static std::vector<std::pair<std::string_view, KeyboardKey>> getKeyboardKeys() noexcept;
	};

	class Mouse;

	enum class LuaMouseListenerType
	{
		Position,
		Scroll,
		Button
	};

	class LuaMouse final : public IMouseListener
	{
	public:
		LuaMouse(Mouse& mouse) noexcept;
		~LuaMouse() noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		using ListenerType = LuaMouseListenerType;

		std::reference_wrapper<Mouse> _mouse;
		std::unordered_map<ListenerType, std::vector<sol::protected_function>> _listeners;

		const glm::vec2& getPosition() const noexcept;
		glm::vec2 getPositionDelta() const noexcept;
		const glm::vec2& getScroll() const noexcept;
		glm::vec2 getScrollDelta() const noexcept;
		bool getButton(MouseButton button) const noexcept;
		bool getLeftButton() const noexcept;
		bool getMiddleButton() const noexcept;
		bool getRightButton() const noexcept;
		bool getActive() const noexcept;

		void onMousePositionChange(const glm::vec2& delta, const glm::vec2& absolute) override;
		void onMouseScrollChange(const glm::vec2& delta, const glm::vec2& absolute) override;
		void onMouseButton(MouseButton button, bool down) override;

		void registerListener(ListenerType type, const sol::protected_function& func) noexcept;
		bool unregisterListener(ListenerType type, const sol::protected_function& func) noexcept;
	};

	class Gamepad;

	class LuaGamepad final
	{
	public:
		LuaGamepad(Gamepad& gamepad) noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		std::reference_wrapper<Gamepad> _gamepad;

		bool getButton(const std::string& name) const noexcept;
		const glm::vec3& getLeftStick() const noexcept;
		const glm::vec3& getRightStick() const noexcept;
		bool isConnected() const noexcept;
	};

	class Input;

	class LuaInput final
	{
	public:
		LuaInput(Input& input) noexcept;
		
		static void bind(sol::state_view& lua) noexcept;
	private:
		OptionalRef<Input> _input;
		LuaKeyboard _keyboard;
		LuaMouse _mouse;
		std::vector<LuaGamepad> _gamepads;

		void addBindings(const std::string& name, const sol::table& data) noexcept;
		LuaKeyboard& getKeyboard() noexcept;
		LuaMouse& getMouse() noexcept;
		OptionalRef<LuaGamepad>::std_t getGamepad(uint8_t num = 0) noexcept;
		const std::vector<LuaGamepad>& getGamepads() noexcept;

	};
}