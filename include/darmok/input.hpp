#pragma once

#include <darmok/export.h>
#include <functional>
#include <string>
#include <array>
#include <vector>
#include <variant>
#include <memory>
#include <cstdint>
#include <unordered_set>
#include <darmok/utils.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/input_fwd.hpp>
#include <darmok/glm.hpp>
#include <darmok/utf8.hpp>
#include <bx/bx.h>

namespace darmok
{
	using KeyboardKeys = std::unordered_set<KeyboardKey>;
	using KeyboardModifiers = std::unordered_set<KeyboardModifier>;
	using KeyboardChars = Utf8Vector;
	class KeyboardImpl;

	class DARMOK_EXPORT BX_NO_VTABLE IKeyboardListener
	{
	public:
		virtual ~IKeyboardListener() = default;
		virtual void onKeyboardKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down) {};
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
		[[nodiscard]] const KeyboardKeys& getKeys() const noexcept;
		[[nodiscard]] const KeyboardModifiers& getModifiers() const noexcept;
		[[nodiscard]] const KeyboardChars& getUpdateChars() const noexcept;

		void addListener(std::unique_ptr<IKeyboardListener>&& listener) noexcept;
		void addListener(IKeyboardListener& listener) noexcept;
		bool removeListener(IKeyboardListener& listener) noexcept;

		[[nodiscard]] const KeyboardImpl& getImpl() const noexcept;
		[[nodiscard]] KeyboardImpl& getImpl() noexcept;

		[[nodiscard]] static char keyToAscii(KeyboardKey key, bool upper) noexcept;
		[[nodiscard]] static const std::string& getKeyName(KeyboardKey key) noexcept;
		[[nodiscard]] static const std::string& getModifierName(KeyboardModifier mod) noexcept;
		[[nodiscard]] static std::optional<KeyboardKey> readKey(std::string_view name) noexcept;
		[[nodiscard]] static std::optional<KeyboardModifier> readModifier(std::string_view name) noexcept;

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

		[[nodiscard]] const glm::vec2& getPosition() const noexcept;
		[[nodiscard]] const glm::vec2& getVelocity() const noexcept;
		[[nodiscard]] const glm::vec2& getScroll() const noexcept;
		[[nodiscard]] bool getActive() const noexcept;
		[[nodiscard]] const MouseButtons& getButtons() const noexcept;
		[[nodiscard]] bool getButton(MouseButton button) const noexcept;

		void addListener(std::unique_ptr<IMouseListener>&& listener) noexcept;
		void addListener(IMouseListener& listener) noexcept;
		bool removeListener(IMouseListener& listener) noexcept;

		[[nodiscard]] const MouseImpl& getImpl() const noexcept;
		[[nodiscard]] MouseImpl& getImpl() noexcept;

		[[nodiscard]] static std::optional<MouseButton> readButton(std::string_view name) noexcept;
		[[nodiscard]] static const std::string& getButtonName(MouseButton button) noexcept;
		[[nodiscard]] static std::optional<MouseAnalog> readAnalog(std::string_view name) noexcept;
		[[nodiscard]] static const std::string& getAnalogName(MouseAnalog analog) noexcept;

	private:
		std::unique_ptr<MouseImpl> _impl;
	};

	using GamepadButtons = std::array<bool, to_underlying(GamepadButton::Count)>;
	using GamepadSticks = std::array<glm::vec3, to_underlying(GamepadButton::Count)>;

	class DARMOK_EXPORT BX_NO_VTABLE IGamepadListener
	{
	public:
		virtual ~IGamepadListener() = default;
		virtual void onGamepadStickChange(uint8_t num, GamepadStick stick, const glm::vec3& delta, const glm::vec3& absolute) {};
		virtual void onGamepadButton(uint8_t num, GamepadButton button, bool down) {};
		virtual void onGamepadConnect(uint8_t num, bool connected) {};
	};

	class GamepadImpl;

	class DARMOK_EXPORT Gamepad final
	{
	public:
		const static uint8_t MaxAmount = 4;
		const static uint8_t Any = 5;

		Gamepad() noexcept;
		~Gamepad() noexcept;
		Gamepad(const Gamepad& other) = delete;
		Gamepad(Gamepad&& other) = delete;
		
		
		[[nodiscard]] const glm::vec3& getStick(GamepadStick stick) const noexcept;
		[[nodiscard]] const GamepadSticks& getSticks() const noexcept;
		[[nodiscard]] bool getButton(GamepadButton button) const noexcept;
		[[nodiscard]] const GamepadButtons& getButtons() const noexcept;
		[[nodiscard]] bool isConnected() const noexcept;

		void addListener(std::unique_ptr<IGamepadListener>&& listener) noexcept;
		void addListener(IGamepadListener& listener) noexcept;
		bool removeListener(IGamepadListener& listener) noexcept;

		[[nodiscard]] const GamepadImpl& getImpl() const noexcept;
		[[nodiscard]] GamepadImpl& getImpl() noexcept;

		[[nodiscard]] static std::optional<GamepadButton> readButton(std::string_view name) noexcept;
		[[nodiscard]] static const std::string& getButtonName(GamepadButton button) noexcept;
		[[nodiscard]] static std::optional<GamepadStick> readStick(std::string_view name) noexcept;
		[[nodiscard]] static const std::string& getStickName(GamepadStick stick) noexcept;

	private:
		std::unique_ptr<GamepadImpl> _impl;
	};

	class Input;

	struct DARMOK_EXPORT KeyboardInputEvent final
	{
		KeyboardKey key = KeyboardKey::Count;
		KeyboardModifiers modifiers = {};

		bool operator==(const KeyboardInputEvent& other) const noexcept;
	};

	struct DARMOK_EXPORT MouseInputEvent final
	{
		MouseButton button = MouseButton::Left;

		bool operator==(const MouseInputEvent& other) const noexcept;
	};

	struct DARMOK_EXPORT GamepadInputEvent final
	{
		GamepadButton button = GamepadButton::A;
		uint8_t gamepad = Gamepad::Any;

		bool operator==(const GamepadInputEvent& other) const noexcept;
	};

	using InputEvent = std::variant<KeyboardInputEvent, MouseInputEvent, GamepadInputEvent>;
	using InputEvents = std::vector<InputEvent>;

	static bool operator==(const InputEvent& a, const InputEvent& b) noexcept;

	struct DARMOK_EXPORT MouseInputDir final
	{
		MouseAnalog analog = MouseAnalog::Position;
		InputDirType type = InputDirType::Count;

		bool operator==(const MouseInputDir& other) const noexcept;
	};

	struct DARMOK_EXPORT GamepadInputDir final
	{
		GamepadStick stick = GamepadStick::Left;
		InputDirType type = InputDirType::Count;
		uint8_t gamepad = Gamepad::Any;

		bool operator==(const GamepadInputDir& other) const noexcept;
	};

	using InputDir = std::variant<MouseInputDir, GamepadInputDir, InputEvent>;
	using InputDirs = std::vector<InputDir>;

	static bool operator==(const InputDir& a, const InputDir& b) noexcept;

	using Gamepads = std::array<Gamepad, Gamepad::MaxAmount>;

	class InputImpl;

	class DARMOK_EXPORT BX_NO_VTABLE IInputEventListener
	{
	public:
		virtual ~IInputEventListener() = default;
		virtual void onInputEvent(const std::string& tag) = 0;
	};

	struct DARMOK_EXPORT InputSensitivity
	{
		float mouse = 1.F;
		float gamepad = 1.F;
		float event = 1.F;
	};

	class DARMOK_EXPORT Input final
	{
	public:
		Input() noexcept;
		~Input() noexcept;
		Input(const Input& other) = delete;
		Input(Input&& other) = delete;

		[[nodiscard]] Keyboard& getKeyboard() noexcept;
		[[nodiscard]] Mouse& getMouse() noexcept;
		[[nodiscard]] OptionalRef<Gamepad> getGamepad(uint8_t num = Gamepad::Any) noexcept;
		[[nodiscard]] Gamepads& getGamepads() noexcept;

		[[nodiscard]] const Keyboard& getKeyboard() const noexcept;
		[[nodiscard]] const Mouse& getMouse() const noexcept;
		[[nodiscard]] OptionalRef<const Gamepad> getGamepad(uint8_t num = Gamepad::Any) const noexcept;
		[[nodiscard]] const Gamepads& getGamepads() const noexcept;
		
		[[nodiscard]] const InputImpl& getImpl() const noexcept;
		[[nodiscard]] InputImpl& getImpl() noexcept;

		bool checkEvent(const InputEvent& ev) const noexcept;
		bool checkEvents(const InputEvents& evs) const noexcept;

		using Sensitivity = InputSensitivity;
		float getAxis(const InputDirs& positive, const InputDirs& negative, const Sensitivity& sensitivity = {}) const noexcept;
		
		void addListener(const std::string& tag, const InputEvents& evs, std::unique_ptr<IInputEventListener>&& listener) noexcept;
		void addListener(const std::string& tag, const InputEvents& evs, IInputEventListener& listener) noexcept;
		bool removeListener(const std::string& tag, IInputEventListener& listener) noexcept;
		bool removeListener(IInputEventListener& listener) noexcept;

		[[nodiscard]] static bool matchesEvent(const InputEvent& condition, const InputEvent& real) noexcept;
		[[nodiscard]] static std::optional<InputEvent> readEvent(std::string_view name) noexcept;
		[[nodiscard]] static std::optional<InputDir> readDir(std::string_view name) noexcept;
		[[nodiscard]] static std::optional<InputDirType> readDirType(std::string_view name) noexcept;
		[[nodiscard]] static const std::string& getDirTypeName(InputDirType type) noexcept;

	private:
		std::unique_ptr<InputImpl> _impl;
	};
}