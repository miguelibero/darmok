#pragma once

#include <functional>
#include <string>
#include <array>
#include <vector>
#include <variant>
#include <memory>
#include <cstdint>
#include <darmok/utils.hpp>

namespace darmok
{
	enum class KeyboardModifier
	{
		None = 0,
		LeftAlt = 0x01,
		RightAlt = 0x02,
		LeftCtrl = 0x04,
		RightCtrl = 0x08,
		LeftShift = 0x10,
		RightShift = 0x20,
		LeftMeta = 0x40,
		RightMeta = 0x80,
	};

	enum class KeyboardKey
	{
		None = 0,
		Esc,
		Return,
		Tab,
		Space,
		Backspace,
		Up,
		Down,
		Left,
		Right,
		Insert,
		Delete,
		Home,
		End,
		PageUp,
		PageDown,
		Print,
		Plus,
		Minus,
		LeftBracket,
		RightBracket,
		Semicolon,
		Quote,
		Comma,
		Period,
		Slash,
		Backslash,
		Tilde,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		NumPad0,
		NumPad1,
		NumPad2,
		NumPad3,
		NumPad4,
		NumPad5,
		NumPad6,
		NumPad7,
		NumPad8,
		NumPad9,
		Key0,
		Key1,
		Key2,
		Key3,
		Key4,
		Key5,
		Key6,
		Key7,
		Key8,
		Key9,
		KeyA,
		KeyB,
		KeyC,
		KeyD,
		KeyE,
		KeyF,
		KeyG,
		KeyH,
		KeyI,
		KeyJ,
		KeyK,
		KeyL,
		KeyM,
		KeyN,
		KeyO,
		KeyP,
		KeyQ,
		KeyR,
		KeyS,
		KeyT,
		KeyU,
		KeyV,
		KeyW,
		KeyX,
		KeyY,
		KeyZ,

		Count
	};

	struct KeyboardModifiers final
	{
		static constexpr uint8_t None = 0;

		static constexpr uint8_t LeftShift = to_underlying(KeyboardModifier::LeftShift);
		static constexpr uint8_t RightShift = to_underlying(KeyboardModifier::RightShift);
		static constexpr uint8_t LeftCtrl = to_underlying(KeyboardModifier::LeftCtrl);
		static constexpr uint8_t RightCtrl = to_underlying(KeyboardModifier::RightCtrl);
		static constexpr uint8_t LeftAlt = to_underlying(KeyboardModifier::LeftAlt);
		static constexpr uint8_t RightAlt = to_underlying(KeyboardModifier::RightAlt);
		static constexpr uint8_t LeftMeta = to_underlying(KeyboardModifier::LeftMeta);
		static constexpr uint8_t RightMeta = to_underlying(KeyboardModifier::RightMeta);

		static constexpr uint8_t Shift = LeftShift | RightShift;
		static constexpr uint8_t Ctrl = LeftCtrl | RightCtrl;
		static constexpr uint8_t Alt = LeftAlt | RightAlt;
		static constexpr uint8_t Meta = LeftMeta | RightMeta;
		static constexpr uint8_t Max = Meta;
	};

	struct Utf8Char final
	{
		uint32_t data;
		uint8_t len;

		Utf8Char(uint32_t data = 0, uint8_t len = 0);
	};

	typedef std::array<uint32_t, to_underlying(KeyboardKey::Count)> KeyboardKeys;

	class InputImpl;
	class KeyboardImpl;

	class Keyboard final
	{
	public:
		bool getKey(KeyboardKey key) const;
		bool getKey(KeyboardKey key, uint8_t& modifiers) const;
		const KeyboardKeys& getKeys() const;
		uint8_t getModifiers() const;

		const KeyboardImpl& getImpl() const;
		KeyboardImpl& getImpl();

		static char keyToAscii(KeyboardKey key, uint8_t modifiers);
		static const std::string& getKeyName(KeyboardKey key);
	private:
		Keyboard();
		Keyboard(const Keyboard& other) = delete;
		Keyboard(Keyboard&& other) = delete;

		std::unique_ptr<KeyboardImpl> _impl;

		friend InputImpl;
	};

	enum class MouseButton
	{
		None,
		Left,
		Middle,
		Right,

		Count
	};

	struct MousePosition final
	{
		int32_t x;
		int32_t y;
		int32_t z;

		MousePosition(int32_t x = 0, int32_t y = 0, int32_t z = 0);
	};

	struct RelativeMousePosition final
	{
		float x;
		float y;
		float z;

		RelativeMousePosition(float x = 0.0f, float y = 0.0f, float z = 0.0f);
	};

	typedef std::array<bool, to_underlying(MouseButton::Count)> MouseButtons;

	class MouseImpl;

	class Mouse final
	{
	public:
		static const std::string& getButtonName(MouseButton button);

		const MousePosition& getPosition() const;
		const MouseButtons& getButtons() const;
		bool getLocked() const;
		bool getButton(MouseButton button) const;

		const MouseImpl& getImpl() const;
		MouseImpl& getImpl();

	private:
		Mouse();
		Mouse(const Mouse& other) = delete;
		Mouse(Mouse&& other) = delete;

		std::unique_ptr<MouseImpl> _impl;
		
		friend InputImpl;
	};

	enum class GamepadAxis
	{
		LeftX,
		LeftY,
		LeftZ,
		RightX,
		RightY,
		RightZ,

		Count
	};

	enum class GamepadButton
	{
		None = 0,
		
		A,
		B,
		X,
		Y,
		ThumbL,
		ThumbR,
		ShoulderL,
		ShoulderR,
		Up,
		Down,
		Left,
		Right,
		Back,
		Start,
		Guide,

		Count
	};

	struct GamepadHandle final
	{
		typedef uint16_t idx_t;
		idx_t idx;

		bool operator==(const GamepadHandle& other) const;
		bool operator<(const GamepadHandle& other) const;
		bool isValid() const;
	};

	typedef std::array<bool, to_underlying(GamepadButton::Count)> GamepadButtons;
	typedef std::array<int32_t, to_underlying(GamepadAxis::Count)> GamepadAxes;

	class GamepadImpl;

	class Gamepad final
	{
	public:
		const static GamepadHandle::idx_t MaxAmount = 4;
		static constexpr GamepadHandle DefaultHandle = { 0 };
		static constexpr GamepadHandle InvalidHandle = { UINT16_MAX };
		
		static const std::string& getButtonName(GamepadButton button);
		int32_t getAxis(GamepadAxis axis) const;
		bool getButton(GamepadButton button) const;
		const GamepadAxes& getAxes() const;
		const GamepadButtons& getButtons() const;
		bool isConnected() const;

		const GamepadImpl& getImpl() const;
		GamepadImpl& getImpl();

	private:
		Gamepad();
		Gamepad(const Gamepad& other) = delete;
		Gamepad(Gamepad&& other) = delete;

		std::unique_ptr<GamepadImpl> _impl;
		
		friend InputImpl;
	};

	struct KeyboardBindingKey final
	{
		KeyboardKey key;
		uint8_t modifiers;
	};

	struct MouseBindingKey final
	{
		MouseButton button;
	};

	struct GamepadBindingKey final
	{
		GamepadHandle gamepad;
		GamepadButton button;
	};

	typedef std::variant<KeyboardBindingKey, MouseBindingKey, GamepadBindingKey> InputBindingKey;

	struct InputBinding final
	{
		static std::size_t hashKey(const InputBindingKey& key);

		InputBindingKey key;
		bool once;
		std::function<void()> fn;
	};

	typedef std::array<Gamepad, Gamepad::MaxAmount> Gamepads;

	struct InputState final
	{
		RelativeMousePosition mouse;
		std::vector<Utf8Char> chars;
	};

	class Input final
	{
	public:
		void process();

		void addBindings(const std::string& name, std::vector<InputBinding>&& bindings);
		void removeBindings(const std::string& name);

		Keyboard& getKeyboard();
		Mouse& getMouse();
		Gamepad& getGamepad(const GamepadHandle& handle = Gamepad::DefaultHandle);
		Gamepads& getGamepads();

		const Keyboard& getKeyboard() const;
		const Mouse& getMouse() const;
		const Gamepad& getGamepad(const GamepadHandle& handle = Gamepad::DefaultHandle) const;
		const Gamepads& getGamepads() const;

		static Input& get();
		const InputImpl& getImpl() const;
		InputImpl& getImpl();

	private:
		Input();
		Input(const Input& other) = delete;
		Input(Input&& other) = delete;

		std::unique_ptr<InputImpl> _impl;
	};
}

template<> struct std::hash<darmok::InputBindingKey> {
	std::size_t operator()(darmok::InputBindingKey const& key) const noexcept {
		return darmok::InputBinding::hashKey(key);
	}
};