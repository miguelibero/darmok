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
	enum class KeyboardModifier : uint8_t
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

	enum class KeyboardModifierGroup : uint8_t
	{
		None = 0,
		Alt = to_underlying(KeyboardModifier::LeftAlt) | to_underlying(KeyboardModifier::RightAlt),
		Ctrl = to_underlying(KeyboardModifier::LeftCtrl) | to_underlying(KeyboardModifier::RightCtrl),
		Shift = to_underlying(KeyboardModifier::LeftShift) | to_underlying(KeyboardModifier::RightShift),
		Meta = to_underlying(KeyboardModifier::LeftMeta) | to_underlying(KeyboardModifier::RightMeta),
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
	using KeyboardChars = std::vector<Utf8Char>;
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
		[[nodiscard]] const KeyboardChars& getUpdateChars() const noexcept;
		[[nodiscard]] const KeyboardImpl& getImpl() const noexcept;
		[[nodiscard]] KeyboardImpl& getImpl() noexcept;

		[[nodiscard]] static char keyToAscii(KeyboardKey key, uint8_t modifiers) noexcept;
		[[nodiscard]] static const std::string& getKeyName(KeyboardKey key) noexcept;

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

	using MouseButtons = std::array<bool, to_underlying(MouseButton::Count)>;

	class MouseImpl;

	class Mouse final
	{
	public:
		Mouse() noexcept;
		Mouse(const Mouse& other) = delete;
		Mouse(Mouse&& other) = delete;

		[[nodiscard]] static const std::string& getButtonName(MouseButton button) noexcept;

		[[nodiscard]] const glm::vec2& getPosition() const noexcept;
		[[nodiscard]] const glm::vec2& getScroll() const noexcept;
		[[nodiscard]] const MouseButtons& getButtons() const noexcept;
		[[nodiscard]] bool getButton(MouseButton button) const noexcept;

		[[nodiscard]] const MouseImpl& getImpl() const noexcept;
		[[nodiscard]] MouseImpl& getImpl() noexcept;

	private:
		std::unique_ptr<MouseImpl> _impl;
	};

	enum class GamepadStick
	{
		Left,
		Right,
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
	using GamepadSticks = std::array<glm::ivec3, to_underlying(GamepadButton::Count)>;

	class GamepadImpl;

	class Gamepad final
	{
	public:
		const static uint8_t MaxAmount = 4;

		Gamepad() noexcept;
		Gamepad(const Gamepad& other) = delete;
		Gamepad(Gamepad&& other) = delete;
		
		[[nodiscard]] static const std::string& getButtonName(GamepadButton button) noexcept;
		
		[[nodiscard]] const glm::ivec3& getStick(GamepadStick stick) const noexcept;
		[[nodiscard]] const GamepadSticks& getSticks() const noexcept;
		[[nodiscard]] bool getButton(GamepadButton button) const noexcept;
		[[nodiscard]] const GamepadButtons& getButtons() const noexcept;
		[[nodiscard]] bool isConnected() const noexcept;

		[[nodiscard]] const GamepadImpl& getImpl() const noexcept;
		[[nodiscard]] GamepadImpl& getImpl() noexcept;

	private:
		std::unique_ptr<GamepadImpl> _impl;
	};

	struct KeyboardBindingKey final
	{
		KeyboardKey key;
		uint8_t modifiers;

		size_t hash() const noexcept;

		static std::optional<KeyboardKey> readKey(std::string_view name) noexcept;
		static uint8_t readModifiers(std::string_view name) noexcept;
		static std::optional<KeyboardBindingKey> read(std::string_view name) noexcept;
	};

	struct MouseBindingKey final
	{
		MouseButton button;

		size_t hash() const noexcept;

		static std::optional<MouseButton> readButton(std::string_view name) noexcept;
		static std::optional<MouseBindingKey> read(std::string_view name) noexcept;
	};

	struct GamepadBindingKey final
	{
		uint8_t gamepad;
		GamepadButton button;

		size_t hash() const noexcept;

		static std::optional<GamepadButton> readButton(std::string_view name) noexcept;
		static std::optional<GamepadBindingKey> read(std::string_view name) noexcept;
	};

	using InputBindingKey = std::variant<KeyboardBindingKey, MouseBindingKey, GamepadBindingKey>;

	struct InputBinding final
	{
		static std::size_t hashKey(const InputBindingKey& key) noexcept;

		InputBindingKey key;
		bool once;
		std::function<void()> fn;

		static std::optional<InputBindingKey> readKey(std::string_view name) noexcept;
		static std::optional<InputBinding> read(std::string_view name, std::function<void()>&& fn) noexcept;
	};

	using Gamepads = std::array<Gamepad, Gamepad::MaxAmount>;

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

		[[nodiscard]] Keyboard& getKeyboard() noexcept;
		[[nodiscard]] Mouse& getMouse() noexcept;
		[[nodiscard]] OptionalRef<Gamepad> getGamepad(uint8_t num = 0) noexcept;
		[[nodiscard]] Gamepads& getGamepads() noexcept;

		[[nodiscard]] const Keyboard& getKeyboard() const noexcept;
		[[nodiscard]] const Mouse& getMouse() const noexcept;
		[[nodiscard]] OptionalRef<const Gamepad> getGamepad(uint8_t num = 0) const noexcept;
		[[nodiscard]] const Gamepads& getGamepads() const noexcept;
		
		[[nodiscard]] const InputImpl& getImpl() const noexcept;
		[[nodiscard]] InputImpl& getImpl() noexcept;

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
