#pragma once

#include <functional>
#include <string>
#include <array>
#include <vector>
#include <variant>
#include <memory>
#include <cstdint>
#include <functional>
#include <darmok/utils.hpp>
#include <darmok/optional_ref.hpp>
#include <glm/glm.hpp>

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

	using KeyboardKeys = std::array<uint32_t, to_underlying(KeyboardKey::Count)>;

	class KeyboardImpl;

	class Keyboard final
	{
	public:
		Keyboard() noexcept;
		Keyboard(const Keyboard& other) = delete;
		Keyboard(Keyboard&& other) = delete;

		[[nodiscard]] bool getKey(KeyboardKey key) const noexcept;
		[[nodiscard]] bool getKey(KeyboardKey key, uint8_t& modifiers) const noexcept;
		[[nodiscard]] const KeyboardKeys& getKeys() const noexcept;
		[[nodiscard]] uint8_t getModifiers() const noexcept;

		[[nodiscard]] const KeyboardImpl& getImpl() const noexcept;
		[[nodiscard]] KeyboardImpl& getImpl() noexcept;

		static char keyToAscii(KeyboardKey key, uint8_t modifiers) noexcept;
		static const std::string& getKeyName(KeyboardKey key) noexcept;
	private:
		
		std::unique_ptr<KeyboardImpl> _impl;
	};

	enum class MouseButton
	{
		Left,
		Middle,
		Right,

		Count
	};

	using MousePosition = glm::vec<3, int32_t>;
	using RelativeMousePosition = glm::vec<3, float>;
	using MouseButtons = std::array<bool, to_underlying(MouseButton::Count)>;

	class MouseImpl;

	class Mouse final
	{
	public:
		Mouse() noexcept;
		Mouse(const Mouse& other) = delete;
		Mouse(Mouse&& other) = delete;

		[[nodiscard]] static const std::string& getButtonName(MouseButton button) noexcept;

		[[nodiscard]] const MousePosition& getPosition() const noexcept;
		[[nodiscard]] const MouseButtons& getButtons() const noexcept;
		[[nodiscard]] bool getLocked() const noexcept;
		[[nodiscard]] bool getButton(MouseButton button) const noexcept;

		[[nodiscard]] const MouseImpl& getImpl() const noexcept;
		[[nodiscard]] MouseImpl& getImpl() noexcept;

	private:
		std::unique_ptr<MouseImpl> _impl;
	};

	// TODO: change to 2 glm::vec<3, int32_t>
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

	using GamepadButtons = std::array<bool, to_underlying(GamepadButton::Count)>;
	using GamepadAxes = std::array<int32_t, to_underlying(GamepadAxis::Count)>;

	class GamepadImpl;

	class Gamepad final
	{
	public:
		const static uint8_t MaxAmount = 4;

		Gamepad() noexcept;
		Gamepad(const Gamepad& other) = delete;
		Gamepad(Gamepad&& other) = delete;
		
		static const std::string& getButtonName(GamepadButton button) noexcept;
		int32_t getAxis(GamepadAxis axis) const noexcept;
		bool getButton(GamepadButton button) const noexcept;
		const GamepadAxes& getAxes() const noexcept;
		const GamepadButtons& getButtons() const noexcept;
		bool isConnected() const noexcept;

		const GamepadImpl& getImpl() const noexcept;
		GamepadImpl& getImpl() noexcept;

	private:
		std::unique_ptr<GamepadImpl> _impl;
	};

	struct KeyboardBindingKey final
	{
		KeyboardKey key;
		uint8_t modifiers;

		size_t hash() const noexcept;
	};

	struct MouseBindingKey final
	{
		MouseButton button;

		size_t hash() const noexcept;
	};

	struct GamepadBindingKey final
	{
		uint8_t gamepad;
		GamepadButton button;

		size_t hash() const noexcept;
	};

	using InputBindingKey = std::variant<KeyboardBindingKey, MouseBindingKey, GamepadBindingKey>;

	struct InputBinding final
	{
		static std::size_t hashKey(const InputBindingKey& key) noexcept;

		InputBindingKey key;
		bool once;
		std::function<void()> fn;
	};

	using Gamepads = std::array<Gamepad, Gamepad::MaxAmount>;

	struct InputState final
	{
		RelativeMousePosition mouse;
		std::vector<Utf8Char> chars;
	};

	class InputImpl;

	class Input final
	{
	public:
		Input() noexcept;
		Input(const Input& other) = delete;
		Input(Input&& other) = delete;
		void processBindings() noexcept;

		void addBindings(std::string_view name, std::vector<InputBinding>&& bindings) noexcept;
		void removeBindings(std::string_view name) noexcept;

		Keyboard& getKeyboard() noexcept;
		Mouse& getMouse() noexcept;
		OptionalRef<Gamepad> getGamepad(uint8_t num = 0) noexcept;
		Gamepads& getGamepads() noexcept;

		const Keyboard& getKeyboard() const noexcept;
		const Mouse& getMouse() const noexcept;
		OptionalRef<const Gamepad> getGamepad(uint8_t num = 0) const noexcept;
		const Gamepads& getGamepads() const noexcept;

		const InputImpl& getImpl() const noexcept;
		InputImpl& getImpl() noexcept;

	private:
		std::unique_ptr<InputImpl> _impl;
	};
}

template<> struct std::hash<darmok::KeyboardBindingKey>
{
	std::size_t operator()(darmok::KeyboardBindingKey const& key) const noexcept
	{
		return key.hash();
	}
};

template<> struct std::equal_to<darmok::KeyboardBindingKey>
{
	bool operator()(const darmok::KeyboardBindingKey& lhs, const darmok::KeyboardBindingKey& rhs) const
	{
		return lhs.key == rhs.key && lhs.modifiers == rhs.modifiers;
	}
};

template<> struct std::hash<darmok::MouseBindingKey>
{
	std::size_t operator()(darmok::MouseBindingKey const& key) const noexcept
	{
		return key.hash();
	}
};

template<> struct std::equal_to<darmok::MouseBindingKey>
{
	bool operator()(const darmok::MouseBindingKey& lhs, const darmok::MouseBindingKey& rhs) const
	{
		return lhs.button == rhs.button;
	}
};

template<> struct std::hash<darmok::GamepadBindingKey>
{
	std::size_t operator()(darmok::GamepadBindingKey const& key) const noexcept
	{
		return key.hash();
	}
};

template<> struct std::equal_to<darmok::GamepadBindingKey>
{
	bool operator()(const darmok::GamepadBindingKey& lhs, const darmok::GamepadBindingKey& rhs) const
	{
		return lhs.gamepad == rhs.gamepad && lhs.button == rhs.button;
	}
};


template<> struct std::equal_to<darmok::InputBindingKey>
{
	bool operator()(const darmok::InputBindingKey& lhs, const darmok::InputBindingKey& rhs) const
	{
		return std::hash<darmok::InputBindingKey>{}(lhs) == std::hash<darmok::InputBindingKey>{}(rhs);
	}
};
