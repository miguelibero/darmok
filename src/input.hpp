#pragma once

#include <darmok/input.hpp>
#include <darmok/utils.hpp>

#include <string>
#include <array>
#include <unordered_map>
#include <unordered_set>

namespace darmok
{

#pragma region Keyboard

	class KeyboardImpl final
	{
	public:
		KeyboardImpl() noexcept;
		KeyboardImpl(const KeyboardImpl& other) = delete;
		KeyboardImpl(KeyboardImpl&& other) = delete;

		[[nodiscard]] bool getKey(KeyboardKey key) const noexcept;
		[[nodiscard]] bool getKey(KeyboardKey key, uint8_t& modifiers) const noexcept;
		[[nodiscard]] const KeyboardKeys& getKeys() const noexcept;
		[[nodiscard]] uint8_t getModifiers() const noexcept;
		[[nodiscard]] const KeyboardChars& getUpdateChars() const noexcept;

		void addListener(IKeyboardListener& listener) noexcept;
		bool removeListener(IKeyboardListener& listener) noexcept;

		void flush() noexcept;
		void reset() noexcept;
		bool setKey(KeyboardKey key, uint8_t modifiers, bool down) noexcept;
		void pushChar(const Utf8Char& data) noexcept;

		void update() noexcept;

	private:
		static uint32_t encodeKey(bool down, uint8_t modifiers) noexcept;
		static bool decodeKey(uint32_t state, uint8_t& modifiers) noexcept;

		Utf8Char popChar() noexcept;

		KeyboardKeys _keys;
		std::array<Utf8Char, 256> _chars;
		KeyboardChars _updateChars;
		size_t _charsRead;
		size_t _charsWrite;
		std::unordered_set<OptionalRef<IKeyboardListener>> _listeners;
	};

#pragma endregion Keyboard

#pragma region Mouse

	class MouseImpl final
	{
	public:
		MouseImpl() noexcept;
		MouseImpl(const MouseImpl& other) = delete;
		MouseImpl(MouseImpl&& other) = delete;

		[[nodiscard]] bool getButton(MouseButton button) const noexcept;
		[[nodiscard]] bool getActive() const noexcept;
		[[nodiscard]] const glm::vec2& getPosition() const noexcept;
		[[nodiscard]] glm::vec2 getPositionDelta() const noexcept;
		[[nodiscard]] const glm::vec2& getScroll() const noexcept;
		[[nodiscard]] glm::vec2 getScrollDelta() const noexcept;
		[[nodiscard]] const MouseButtons& getButtons() const noexcept;

		void addListener(IMouseListener& listener) noexcept;
		bool removeListener(IMouseListener& listener) noexcept;

		bool setActive(bool active) noexcept;
		bool setPosition(const glm::vec2& pos) noexcept;
		bool setScroll(const glm::vec2& scroll) noexcept;
		bool setButton(MouseButton button, bool down) noexcept;

		void update() noexcept;

	private:
		glm::vec2 _position;
		glm::vec2 _lastPosition;
		glm::vec2 _scroll;
		glm::vec2 _lastScroll;

		MouseButtons _buttons;
		bool _active;
		bool _hasBeenInactive;
		std::unordered_set<OptionalRef<IMouseListener>> _listeners;
	};

#pragma endregion Mouse

#pragma region Gamepad

	class GamepadImpl;

	class GamepadImpl final
	{
	public:
		GamepadImpl() noexcept;
		GamepadImpl(const GamepadImpl& other) = delete;
		GamepadImpl(GamepadImpl&& other) = delete;

		[[nodiscard]] const glm::ivec3& getStick(GamepadStick stick) const noexcept;
		[[nodiscard]] const GamepadSticks& getSticks() const noexcept;
		[[nodiscard]] bool getButton(GamepadButton button) const noexcept;
		[[nodiscard]] const GamepadButtons& getButtons() const noexcept;
		[[nodiscard]] bool isConnected() const noexcept;

		void addListener(IGamepadListener& listener) noexcept;
		bool removeListener(IGamepadListener& listener) noexcept;

		bool setNumber(uint8_t num) noexcept;
		bool setConnected(bool value) noexcept;

		bool setStick(GamepadStick stick, const glm::ivec3& value) noexcept;
		bool setButton(GamepadButton button, bool down) noexcept;
	private:
		uint8_t _num;
		bool _connected;
		GamepadButtons _buttons;
		GamepadSticks _sticks;
		std::unordered_set<OptionalRef<IGamepadListener>> _listeners;

		void clear() noexcept;
	};

#pragma endregion Gamepad

#pragma region Input

	class InputImpl final
	{
	public:
		InputImpl() noexcept;
		InputImpl(const InputImpl& other) = delete;
		InputImpl(InputImpl&& other) = delete;

		void processBindings() noexcept;
		void resetOnceBindings() noexcept;
		void addBindings(std::string_view name, std::vector<InputBinding>&& bindings) noexcept;
		void removeBindings(std::string_view name) noexcept;
		void clearBindings() noexcept;
		Keyboard& getKeyboard() noexcept;
		Mouse& getMouse() noexcept;
		OptionalRef<Gamepad> getGamepad(uint8_t num) noexcept;
		Gamepads& getGamepads() noexcept;

		void update() noexcept;

	private:
		bool bindingTriggered(InputBinding& binding) noexcept;
		void processBinding(InputBinding& binding) noexcept;

		Keyboard _keyboard;
		Mouse _mouse;
		Gamepads _gamepads;

		std::unordered_map<std::string, std::vector<InputBinding>> _bindings;
		std::unordered_set<InputBindingKey> _bindingOnce;
	};

#pragma endregion Input

}
