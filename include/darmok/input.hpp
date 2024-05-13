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
#include <darmok/input_fwd.hpp>
#include <glm/glm.hpp>
#include <bx/bx.h>

namespace darmok
{
	using KeyboardKeys = std::array<uint32_t, to_underlying(KeyboardKey::Count)>;
	using KeyboardChars = std::vector<Utf8Char>;
	class KeyboardImpl;

	class BX_NO_VTABLE IKeyboardListener
	{
	public:
		DLLEXPORT virtual ~IKeyboardListener() = default;
		DLLEXPORT virtual void onKeyboardKey(KeyboardKey key, uint8_t modifiers, bool down) {};
		DLLEXPORT virtual void onKeyboardChar(const Utf8Char& chr) {};
	};

	class Keyboard final
	{
	public:
		Keyboard() noexcept;
		Keyboard(const Keyboard& other) = delete;
		Keyboard(Keyboard&& other) = delete;

		[[nodiscard]] DLLEXPORT bool getKey(KeyboardKey key) const noexcept;
		[[nodiscard]] DLLEXPORT bool getKey(KeyboardKey key, uint8_t& modifiers) const noexcept;
		[[nodiscard]] DLLEXPORT const KeyboardKeys& getKeys() const noexcept;
		[[nodiscard]] DLLEXPORT uint8_t getModifiers() const noexcept;
		[[nodiscard]] DLLEXPORT const KeyboardChars& getUpdateChars() const noexcept;

		DLLEXPORT void addListener(IKeyboardListener& listener) noexcept;
		DLLEXPORT bool removeListener(IKeyboardListener& listener) noexcept;

		[[nodiscard]] const KeyboardImpl& getImpl() const noexcept;
		[[nodiscard]] KeyboardImpl& getImpl() noexcept;

		[[nodiscard]] DLLEXPORT static char keyToAscii(KeyboardKey key, uint8_t modifiers) noexcept;
		[[nodiscard]] DLLEXPORT static const std::string& getKeyName(KeyboardKey key) noexcept;


	private:
		std::unique_ptr<KeyboardImpl> _impl;
	};

	using MouseButtons = std::array<bool, to_underlying(MouseButton::Count)>;

	class MouseImpl;

	class BX_NO_VTABLE IMouseListener
	{
	public:
		DLLEXPORT virtual ~IMouseListener() = default;
		DLLEXPORT virtual void onMouseActive(bool active) {};
		DLLEXPORT virtual void onMousePositionChange(const glm::vec2& delta) {};
		DLLEXPORT virtual void onMouseScrollChange(const glm::vec2& delta) {};
		DLLEXPORT virtual void onMouseButton(MouseButton button, bool down) {};
	};

	class Mouse final
	{
	public:
		Mouse() noexcept;
		Mouse(const Mouse& other) = delete;
		Mouse(Mouse&& other) = delete;

		[[nodiscard]] DLLEXPORT static const std::string& getButtonName(MouseButton button) noexcept;

		[[nodiscard]] DLLEXPORT const glm::vec2& getPosition() const noexcept;
		[[nodiscard]] DLLEXPORT glm::vec2 getPositionDelta() const noexcept;
		[[nodiscard]] DLLEXPORT const glm::vec2& getScroll() const noexcept;
		[[nodiscard]] DLLEXPORT glm::vec2 getScrollDelta() const noexcept;
		[[nodiscard]] DLLEXPORT bool getActive() const noexcept;
		[[nodiscard]] DLLEXPORT const MouseButtons& getButtons() const noexcept;
		[[nodiscard]] DLLEXPORT bool getButton(MouseButton button) const noexcept;

		DLLEXPORT void addListener(IMouseListener& listener) noexcept;
		DLLEXPORT bool removeListener(IMouseListener& listener) noexcept;

		[[nodiscard]] const MouseImpl& getImpl() const noexcept;
		[[nodiscard]] MouseImpl& getImpl() noexcept;

	private:
		std::unique_ptr<MouseImpl> _impl;
	};

	using GamepadButtons = std::array<bool, to_underlying(GamepadButton::Count)>;
	using GamepadSticks = std::array<glm::ivec3, to_underlying(GamepadButton::Count)>;

	class GamepadImpl;

	class BX_NO_VTABLE IGamepadListener
	{
	public:
		DLLEXPORT virtual ~IGamepadListener() = default;
		DLLEXPORT virtual void onGamepadStickChange(uint8_t num, GamepadStick stick, const glm::ivec3& delta) {};
		DLLEXPORT virtual void onGamepadButton(uint8_t num, GamepadButton button, bool down) {};
		DLLEXPORT virtual void onGamepadConnect(uint8_t num, bool connected) {};
	};

	class Gamepad final
	{
	public:
		const static uint8_t MaxAmount = 4;

		Gamepad() noexcept;
		Gamepad(const Gamepad& other) = delete;
		Gamepad(Gamepad&& other) = delete;
		
		[[nodiscard]] DLLEXPORT static const std::string& getButtonName(GamepadButton button) noexcept;
		
		[[nodiscard]] DLLEXPORT const glm::ivec3& getStick(GamepadStick stick) const noexcept;
		[[nodiscard]] DLLEXPORT const GamepadSticks& getSticks() const noexcept;
		[[nodiscard]] DLLEXPORT bool getButton(GamepadButton button) const noexcept;
		[[nodiscard]] DLLEXPORT const GamepadButtons& getButtons() const noexcept;
		[[nodiscard]] DLLEXPORT bool isConnected() const noexcept;

		DLLEXPORT void addListener(IGamepadListener& listener) noexcept;
		DLLEXPORT bool removeListener(IGamepadListener& listener) noexcept;

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

		DLLEXPORT void processBindings() noexcept;
		DLLEXPORT void addBindings(std::string_view name, std::vector<InputBinding>&& bindings) noexcept;
		DLLEXPORT void removeBindings(std::string_view name) noexcept;

		[[nodiscard]] DLLEXPORT Keyboard& getKeyboard() noexcept;
		[[nodiscard]] DLLEXPORT Mouse& getMouse() noexcept;
		[[nodiscard]] DLLEXPORT OptionalRef<Gamepad> getGamepad(uint8_t num = 0) noexcept;
		[[nodiscard]] DLLEXPORT Gamepads& getGamepads() noexcept;

		[[nodiscard]] DLLEXPORT const Keyboard& getKeyboard() const noexcept;
		[[nodiscard]] DLLEXPORT const Mouse& getMouse() const noexcept;
		[[nodiscard]] DLLEXPORT OptionalRef<const Gamepad> getGamepad(uint8_t num = 0) const noexcept;
		[[nodiscard]] DLLEXPORT const Gamepads& getGamepads() const noexcept;
		
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
