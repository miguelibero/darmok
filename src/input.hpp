#pragma once

#include <darmok/input.hpp>
#include <darmok/utils.hpp>

#include <string>
#include <unordered_map>
#include <array>
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

		bool getKey(KeyboardKey key) const noexcept;
		bool getKey(KeyboardKey key, uint8_t& modifiers) const noexcept;
		const KeyboardKeys& getKeys() const noexcept;
		uint8_t getModifiers() const noexcept;
		Utf8Char popChar() noexcept;
		void flush() noexcept;

		void reset() noexcept;
		void setKey(KeyboardKey key, uint8_t modifiers, bool down) noexcept;
		void pushChar(const Utf8Char& data) noexcept;

	private:
		static uint32_t encodeKey(bool down, uint8_t modifiers) noexcept;
		static bool decodeKey(uint32_t state, uint8_t& modifiers) noexcept;

		KeyboardKeys _keys;
		std::array<Utf8Char, 256> _chars;
		size_t _charsRead;
		size_t _charsWrite;
	};

#pragma endregion Keyboard

#pragma region Mouse

	class MouseImpl final
	{
	public:
		MouseImpl() noexcept;
		MouseImpl(const MouseImpl& other) = delete;
		MouseImpl(MouseImpl&& other) = delete;

		[[nodiscard]] void setWheelDelta(uint16_t wheelDelta) noexcept;
		[[nodiscard]] bool getButton(MouseButton button) const noexcept;
		[[nodiscard]] RelativeMousePosition popRelativePosition() noexcept;
		[[nodiscard]] const MousePosition& getPosition() const noexcept;
		[[nodiscard]] const MouseButtons& getButtons() const noexcept;
		[[nodiscard]] bool getLocked() const noexcept;

		bool setLocked(bool lock) noexcept;
		void setResolution(const glm::uvec2& size) noexcept;
		void setPosition(const MousePosition& pos) noexcept;
		void setButton(MouseButton button, bool down) noexcept;

	private:
		MousePosition _absolute;
		RelativeMousePosition _relative;

		MouseButtons _buttons;
		glm::uvec2 _size;
		uint16_t _wheelDelta;
		bool _lock;
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

		[[nodiscard]] int32_t getAxis(GamepadAxis key) const noexcept;
		[[nodiscard]] bool getButton(GamepadButton button) const noexcept;
		[[nodiscard]] const GamepadButtons& getButtons() const noexcept;
		[[nodiscard]] const GamepadAxes& getAxes() const noexcept;
		[[nodiscard]] bool isConnected() const noexcept;

		void init(uint8_t num) noexcept;
		void reset() noexcept;
		void setAxis(GamepadAxis axis, int32_t value) noexcept;
		void setButton(GamepadButton button, bool down) noexcept;
	private:
		uint8_t _num;
		GamepadButtons _buttons;
		GamepadAxes _axes;
	};

#pragma endregion Gamepad

#pragma region Input

	class InputImpl final
	{
	public:
		InputImpl() noexcept;
		InputImpl(const InputImpl& other) = delete;
		InputImpl(InputImpl&& other) = delete;

		void process() noexcept;
		void reset() noexcept;
		void addBindings(std::string_view name, std::vector<InputBinding>&& bindings) noexcept;
		void removeBindings(std::string_view name) noexcept;
		Keyboard& getKeyboard() noexcept;
		Mouse& getMouse() noexcept;
		OptionalRef<Gamepad> getGamepad(uint8_t num) noexcept;
		Gamepads& getGamepads() noexcept;

		void update() noexcept;
		const InputState& getState() const noexcept;

	private:
		bool bindingTriggered(InputBinding& binding) noexcept;
		void processBinding(InputBinding& binding) noexcept;

		Keyboard _keyboard;
		Mouse _mouse;
		Gamepads _gamepads;
		InputState _state;

		std::unordered_map<std::string, std::vector<InputBinding>> _bindings;
		std::unordered_set<InputBindingKey> _bindingOnce;
	};

#pragma endregion Input

}
