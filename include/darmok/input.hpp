#pragma once

#include <darmok/export.h>
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
#include <darmok/glm.hpp>
#include <darmok/string.hpp>
#include <bx/bx.h>

namespace darmok
{
	using KeyboardKeys = std::array<uint32_t, to_underlying(KeyboardKey::Count)>;
	using KeyboardChars = std::vector<Utf8Char>;
	class KeyboardImpl;

	class DARMOK_EXPORT BX_NO_VTABLE IKeyboardListener
	{
	public:
		virtual ~IKeyboardListener() = default;
		virtual void onKeyboardKey(KeyboardKey key, uint8_t modifiers, bool down) {};
		virtual void onKeyboardChar(const Utf8Char& chr) {};
	};

	class DARMOK_EXPORT Keyboard final
	{
	public:
		Keyboard() noexcept;
		~Keyboard() noexcept;
		Keyboard(const Keyboard& other) = delete;
		Keyboard(Keyboard&& other) = delete;

		[[nodiscard]] bool getKey(KeyboardKey key) const noexcept;
		[[nodiscard]] bool getKey(KeyboardKey key, uint8_t& modifiers) const noexcept;
		[[nodiscard]] const KeyboardKeys& getKeys() const noexcept;
		[[nodiscard]] uint8_t getModifiers() const noexcept;
		[[nodiscard]] const KeyboardChars& getUpdateChars() const noexcept;

		void addListener(IKeyboardListener& listener) noexcept;
		bool removeListener(IKeyboardListener& listener) noexcept;

		[[nodiscard]] const KeyboardImpl& getImpl() const noexcept;
		[[nodiscard]] KeyboardImpl& getImpl() noexcept;

		[[nodiscard]] static char keyToAscii(KeyboardKey key, uint8_t modifiers) noexcept;
		[[nodiscard]] static const std::string& getKeyName(KeyboardKey key) noexcept;


	private:
		std::unique_ptr<KeyboardImpl> _impl;
	};

	using MouseButtons = std::array<bool, to_underlying(MouseButton::Count)>;

	class MouseImpl;

	class DARMOK_EXPORT BX_NO_VTABLE IMouseListener
	{
	public:
		virtual ~IMouseListener() = default;
		virtual void onMouseActive(bool active) {};
		virtual void onMousePositionChange(const glm::vec2& delta, const glm::vec2& absolute) {};
		virtual void onMouseScrollChange(const glm::vec2& delta, const glm::vec2& absolute) {};
		virtual void onMouseButton(MouseButton button, bool down) {};
	};

	class DARMOK_EXPORT Mouse final
	{
	public:
		Mouse() noexcept;
		~Mouse() noexcept;
		Mouse(const Mouse& other) = delete;
		Mouse(Mouse&& other) = delete;

		[[nodiscard]] static const std::string& getButtonName(MouseButton button) noexcept;

		[[nodiscard]] const glm::vec2& getPosition() const noexcept;
		[[nodiscard]] glm::vec2 getPositionDelta() const noexcept;
		[[nodiscard]] const glm::vec2& getScroll() const noexcept;
		[[nodiscard]] glm::vec2 getScrollDelta() const noexcept;
		[[nodiscard]] bool getActive() const noexcept;
		[[nodiscard]] const MouseButtons& getButtons() const noexcept;
		[[nodiscard]] bool getButton(MouseButton button) const noexcept;

		void addListener(IMouseListener& listener) noexcept;
		bool removeListener(IMouseListener& listener) noexcept;

		[[nodiscard]] const MouseImpl& getImpl() const noexcept;
		[[nodiscard]] MouseImpl& getImpl() noexcept;

	private:
		std::unique_ptr<MouseImpl> _impl;
	};

	using GamepadButtons = std::array<bool, to_underlying(GamepadButton::Count)>;
	using GamepadSticks = std::array<glm::vec3, to_underlying(GamepadButton::Count)>;

	class GamepadImpl;

	class DARMOK_EXPORT BX_NO_VTABLE IGamepadListener
	{
	public:
		virtual ~IGamepadListener() = default;
		virtual void onGamepadStickChange(uint8_t num, GamepadStick stick, const glm::vec3& delta, const glm::vec3& absolute) {};
		virtual void onGamepadButton(uint8_t num, GamepadButton button, bool down) {};
		virtual void onGamepadConnect(uint8_t num, bool connected) {};
	};

	class DARMOK_EXPORT Gamepad final
	{
	public:
		const static uint8_t MaxAmount = 4;

		Gamepad() noexcept;
		~Gamepad() noexcept;
		Gamepad(const Gamepad& other) = delete;
		Gamepad(Gamepad&& other) = delete;
		
		[[nodiscard]] static const std::string& getButtonName(GamepadButton button) noexcept;
		
		[[nodiscard]] const glm::vec3& getStick(GamepadStick stick) const noexcept;
		[[nodiscard]] const GamepadSticks& getSticks() const noexcept;
		[[nodiscard]] bool getButton(GamepadButton button) const noexcept;
		[[nodiscard]] const GamepadButtons& getButtons() const noexcept;
		[[nodiscard]] bool isConnected() const noexcept;

		void addListener(IGamepadListener& listener) noexcept;
		bool removeListener(IGamepadListener& listener) noexcept;

		[[nodiscard]] const GamepadImpl& getImpl() const noexcept;
		[[nodiscard]] GamepadImpl& getImpl() noexcept;

	private:
		std::unique_ptr<GamepadImpl> _impl;
	};

	struct DARMOK_EXPORT KeyboardBindingKey final
	{
		KeyboardKey key;
		uint8_t modifiers;

		size_t hash() const noexcept;

		static std::optional<KeyboardKey> readKey(std::string_view name) noexcept;
		static uint8_t readModifiers(std::string_view name) noexcept;
		static std::optional<KeyboardBindingKey> read(std::string_view name) noexcept;
	};

	struct DARMOK_EXPORT MouseBindingKey final
	{
		MouseButton button;

		size_t hash() const noexcept;

		static std::optional<MouseButton> readButton(std::string_view name) noexcept;
		static std::optional<MouseBindingKey> read(std::string_view name) noexcept;
	};

	struct DARMOK_EXPORT GamepadBindingKey final
	{
		GamepadButton button;
		uint8_t gamepad = -1;

		size_t hash() const noexcept;

		static std::optional<GamepadButton> readButton(std::string_view name) noexcept;
		static std::optional<GamepadBindingKey> read(std::string_view name) noexcept;
	};

	using InputBindingKey = std::variant<KeyboardBindingKey, MouseBindingKey, GamepadBindingKey>;

	struct DARMOK_EXPORT InputBinding final
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

	class DARMOK_EXPORT Input final
	{
	public:
		Input() noexcept;
		~Input() noexcept;
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
