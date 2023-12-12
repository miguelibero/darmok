#pragma once

#include <darmok/input.hpp>
#include <darmok/window.hpp>
#include <darmok/utils.hpp>

#include <string>
#include <unordered_map>
#include <array>

namespace darmok
{

#pragma region Keyboard

	class KeyboardImpl final
	{
	public:
		KeyboardImpl();
		bool getKey(KeyboardKey key) const;
		bool getKey(KeyboardKey key, uint8_t& modifiers) const;
		const KeyboardKeys& getKeys() const;
		uint8_t getModifiers() const;
		Utf8Char popChar();
		void flush();

		void reset();
		void setKey(KeyboardKey key, uint8_t modifiers, bool down);
		void pushChar(const Utf8Char& data);

	private:

		KeyboardImpl(const KeyboardImpl& other) = delete;
		KeyboardImpl(KeyboardImpl&& other) = delete;

		static uint32_t encodeKey(bool down, uint8_t modifiers);
		static bool decodeKey(uint32_t state, uint8_t& modifiers);

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
		MouseImpl();
		void setWheelDelta(uint16_t wheelDelta);
		bool getButton(MouseButton button) const;
		RelativeMousePosition popRelativePosition();
		const MousePosition& getPosition() const;
		const MouseButtons& getButtons() const;
		bool getLocked() const;
		const WindowHandle& getWindow() const;

		bool setLocked(bool lock);
		void setResolution(const WindowSize& size);
		void setPosition(const MousePosition& pos);
		void setButton(MouseButton button, bool down);
		void setWindow(const WindowHandle& win);

	private:
		MouseImpl(const MouseImpl& other) = delete;
		MouseImpl(MouseImpl&& other) = delete;

		WindowHandle _window;
		MousePosition _absolute;
		RelativeMousePosition _relative;

		MouseButtons _buttons;
		WindowSize _size;
		uint16_t _wheelDelta;
		bool _lock;
	};

#pragma endregion Mouse

#pragma region Gamepad

	class GamepadImpl;

	class GamepadImpl final
	{
	public:
		GamepadImpl();

		int32_t getAxis(GamepadAxis key) const;
		bool getButton(GamepadButton button) const;
		const GamepadButtons& getButtons() const;
		const GamepadAxes& getAxes() const;
		bool isConnected() const;

		void init(const GamepadHandle& handle);
		void reset();
		void setAxis(GamepadAxis axis, int32_t value);
		void setButton(GamepadButton button, bool down);
	private:
		GamepadImpl(const GamepadImpl& other) = delete;
		GamepadImpl(GamepadImpl&& other) = delete;

		GamepadHandle _handle;
		GamepadButtons _buttons;
		GamepadAxes _axes;
	};

#pragma endregion Gamepad

#pragma region Input

	class InputImpl final
	{
	public:
		InputImpl();

		void process();
		void reset();
		void addBindings(const std::string& name, std::vector<InputBinding>&& bindings);
		void removeBindings(const std::string& name);
		Keyboard& getKeyboard();
		Mouse& getMouse();
		Gamepad& getGamepad(const GamepadHandle& handle);
		Gamepads& getGamepads();

		void update();
		const InputState& getState() const;

	private:
		InputImpl(const InputImpl& other) = delete;
		InputImpl(InputImpl&& other) = delete;

		bool bindingTriggered(InputBinding& binding);
		void processBinding(InputBinding& binding);

		Keyboard _keyboard;
		Mouse _mouse;
		Gamepads _gamepads;
		InputState _state;

		std::unordered_map<std::string, std::vector<InputBinding>> _bindings;
		std::unordered_map<std::size_t, bool> _bindingOnce;
	};

#pragma endregion Input

}
