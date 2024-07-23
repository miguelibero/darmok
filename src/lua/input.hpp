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

	enum class LuaKeyboardListenerType
	{
		Key,
		Char
	};

	class LuaKeyboard final : public IKeyboardListener
	{
	public:
		LuaKeyboard(Keyboard& kb) noexcept;
		~LuaKeyboard() noexcept;

		void onKeyboardKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down) override;
		void onKeyboardChar(const Utf8Char& chr) override;

		static std::optional<KeyboardKey> readKey(const sol::object& val) noexcept;
		static std::optional<KeyboardModifier> readModifier(const sol::object& val) noexcept;
		static std::optional<KeyboardInputEvent> readEvent(const sol::object& val) noexcept;

		static void bind(sol::state_view& lua) noexcept;

	private:
		using ListenerType = LuaKeyboardListenerType;
		std::reference_wrapper<Keyboard> _kb;
		std::unordered_map<ListenerType, std::vector<sol::protected_function>> _listeners;

		void registerListener(ListenerType type, const sol::protected_function& func) noexcept;
		bool unregisterListener(ListenerType type, const sol::protected_function& func) noexcept;

		bool getKey(KeyboardKey key) const noexcept;
		const KeyboardKeys& getKeys() const noexcept;
		const KeyboardModifiers& getModifiers() const noexcept;

		std::string getUpdateChars() const noexcept;
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

		static std::optional<MouseInputEvent> readEvent(const sol::object& val) noexcept;
		static std::optional<MouseInputDir> readDir(const sol::object& val) noexcept;
		static std::optional<MouseAnalog> readAnalog(const sol::object& val) noexcept;

	private:
		using ListenerType = LuaMouseListenerType;

		std::reference_wrapper<Mouse> _mouse;
		std::unordered_map<ListenerType, std::vector<sol::protected_function>> _listeners;

		const glm::vec2& getPosition() const noexcept;
		const glm::vec2& getVelocity() const noexcept;
		const glm::vec2& getScroll() const noexcept;
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

	enum class LuaGamepadListenerType
	{
		Stick,
		Button,
		Connect
	};

	class Gamepad;

	class LuaGamepad final : public IGamepadListener
	{
	public:
		LuaGamepad(Gamepad& gamepad) noexcept;
		~LuaGamepad() noexcept;
		static void bind(sol::state_view& lua) noexcept;

		static std::optional<GamepadInputEvent> readEvent(const sol::object& val) noexcept;
		static std::optional<GamepadButton> readButton(const sol::object& val) noexcept;
		static std::optional<GamepadStick> readStick(const sol::object& val) noexcept;
		static std::optional<GamepadInputDir> readDir(const sol::object& val) noexcept;

	private:
		using ListenerType = LuaGamepadListenerType;

		std::reference_wrapper<Gamepad> _gamepad;
		std::unordered_map<ListenerType, std::vector<sol::protected_function>> _listeners;

		void onGamepadStickChange(uint8_t num, GamepadStick stick, const glm::vec3& delta, const glm::vec3& absolute) override;
		void onGamepadButton(uint8_t num, GamepadButton button, bool down) override;
		void onGamepadConnect(uint8_t num, bool connected) override;

		void registerListener(ListenerType type, const sol::protected_function& func) noexcept;
		bool unregisterListener(ListenerType type, const sol::protected_function& func) noexcept;

		bool getButton(GamepadButton button) const noexcept;
		const glm::vec3& getLeftStick() const noexcept;
		const glm::vec3& getRightStick() const noexcept;
		bool isConnected() const noexcept;
	};

	class LuaInputEventListener final : public IInputEventListener
	{
	public:
		LuaInputEventListener(const std::string& tag, const sol::protected_function& func) noexcept;
		void onInputEvent(const std::string& tag) override;
		const std::string& getTag() const noexcept;
		const sol::protected_function& getFunc() const noexcept;
	private:
		sol::protected_function _func;
		std::string _tag;
	};

	class Input;

	class LuaInput final
	{
	public:
		LuaInput(Input& input) noexcept;
		~LuaInput() noexcept;
		
		static void bind(sol::state_view& lua) noexcept;

		static std::optional<InputDirType> readDirType(const sol::object& val) noexcept;

	private:
		OptionalRef<Input> _input;
		LuaKeyboard _keyboard;
		LuaMouse _mouse;
		std::vector<LuaGamepad> _gamepads;
		std::vector<LuaInputEventListener> _listeners;

		void registerListener(const std::string& tag, const sol::object& ev, const sol::protected_function& func);
		bool unregisterListener1(const std::string& tag, const sol::protected_function& func) noexcept;
		bool unregisterListener2(const sol::protected_function& func) noexcept;

		LuaKeyboard& getKeyboard() noexcept;
		LuaMouse& getMouse() noexcept;
		OptionalRef<LuaGamepad>::std_t getGamepad(uint8_t num = 0) noexcept;
		const std::vector<LuaGamepad>& getGamepads() noexcept;

		bool checkEvent(const sol::object& ev) const noexcept;
		float getAxis(const sol::object& positive, const sol::object& negative) const noexcept;	

		static std::optional<InputEvent> readEvent(const sol::object& val) noexcept;
		static std::optional<InputDir> readDir(const sol::object& val) noexcept;
		static InputDirs readDirs(const sol::object& val) noexcept;
	};
}