#pragma once

#include <darmok/export.h>
#include <darmok/utils.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/input_fwd.hpp>
#include <darmok/glm.hpp>
#include <darmok/expected.hpp>

#include <functional>
#include <string>
#include <array>
#include <vector>
#include <variant>
#include <memory>
#include <cstdint>
#include <unordered_set>

#include <bx/bx.h>

namespace darmok
{
	using KeyboardKeys = std::unordered_set<KeyboardKey>;
	using KeyboardModifiers = std::unordered_set<KeyboardModifier>;
	class KeyboardImpl;

	class DARMOK_EXPORT BX_NO_VTABLE IKeyboardListener
	{
	public:
		virtual ~IKeyboardListener() = default;
		virtual entt::id_type getKeyboardListenerType() const noexcept { return 0; };
		virtual expected<void, std::string> onKeyboardKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down) noexcept { return {}; };
		virtual expected<void, std::string> onKeyboardChar(char32_t chr) noexcept { return {}; };
	};

	template<typename T>
	class DARMOK_EXPORT BX_NO_VTABLE ITypeKeyboardListener : public IKeyboardListener
	{
	public:
		entt::id_type getKeyboardListenerType() const noexcept override
		{
			return entt::type_hash<T>::value();
		}
	};

	class DARMOK_EXPORT BX_NO_VTABLE IKeyboardListenerFilter
	{
	public:
		virtual ~IKeyboardListenerFilter() = default;
		virtual bool operator()(const IKeyboardListener& listener) const noexcept = 0;
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
		[[nodiscard]] std::u32string_view getUpdateChars() const noexcept;

		void addListener(std::unique_ptr<IKeyboardListener>&& listener) noexcept;
		void addListener(IKeyboardListener& listener) noexcept;
		bool removeListener(const IKeyboardListener& listener) noexcept;
		size_t removeListeners(const IKeyboardListenerFilter& filter) noexcept;

		[[nodiscard]] const KeyboardImpl& getImpl() const noexcept;
		[[nodiscard]] KeyboardImpl& getImpl() noexcept;

		[[nodiscard]] static char keyToAscii(KeyboardKey key, bool upper) noexcept;
		[[nodiscard]] static std::string_view getKeyName(KeyboardKey key) noexcept;
		[[nodiscard]] static std::string_view getModifierName(KeyboardModifier mod) noexcept;
		[[nodiscard]] static std::optional<KeyboardKey> readKey(std::string_view name) noexcept;
		[[nodiscard]] static std::optional<KeyboardModifier> readModifier(std::string_view name) noexcept;

	private:
		std::unique_ptr<KeyboardImpl> _impl;
	};

	using MouseButtons = std::array<bool, toUnderlying(MouseButton::Count)>;

	class MouseImpl;

	class DARMOK_EXPORT BX_NO_VTABLE IMouseListener
	{
	public:
		virtual ~IMouseListener() = default;
		virtual entt::id_type getMouseListenerType() const noexcept { return 0; };
		virtual expected<void, std::string> onMouseActive(bool active) noexcept { return {}; };
		virtual expected<void, std::string> onMousePositionChange(const glm::vec2& delta, const glm::vec2& absolute) noexcept { return {}; };
		virtual expected<void, std::string> onMouseScrollChange(const glm::vec2& delta, const glm::vec2& absolute) noexcept { return {}; };
		virtual expected<void, std::string> onMouseButton(MouseButton button, bool down) noexcept { return {}; };
	};

	template<typename T>
	class DARMOK_EXPORT BX_NO_VTABLE ITypeMouseListener : public IMouseListener
	{
	public:
		entt::id_type getMouseListenerType() const noexcept override
		{
			return entt::type_hash<T>::value();
		}
	};

	class DARMOK_EXPORT BX_NO_VTABLE IMouseListenerFilter
	{
	public:
		virtual ~IMouseListenerFilter() = default;
		virtual bool operator()(const IMouseListener& listener) const = 0;
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
		bool removeListener(const IMouseListener& listener) noexcept;
		size_t removeListeners(const IMouseListenerFilter& filter) noexcept;

		[[nodiscard]] const MouseImpl& getImpl() const noexcept;
		[[nodiscard]] MouseImpl& getImpl() noexcept;

		[[nodiscard]] static std::optional<MouseButton> readButton(std::string_view name) noexcept;
		[[nodiscard]] static std::string_view getButtonName(MouseButton button) noexcept;
		[[nodiscard]] static std::optional<MouseAnalog> readAnalog(std::string_view name) noexcept;
		[[nodiscard]] static std::string_view getAnalogName(MouseAnalog analog) noexcept;

	private:
		std::unique_ptr<MouseImpl> _impl;
	};

	using GamepadButtons = std::array<bool, toUnderlying(GamepadButton::Count)>;
	using GamepadSticks = std::array<glm::vec3, toUnderlying(GamepadButton::Count)>;

	class DARMOK_EXPORT BX_NO_VTABLE IGamepadListener
	{
	public:
		virtual ~IGamepadListener() = default;
		virtual entt::id_type getGamepadListenerType() const noexcept { return 0; };
		virtual expected<void, std::string> onGamepadStickChange(uint8_t num, GamepadStick stick, const glm::vec3& delta, const glm::vec3& absolute) noexcept { return {}; };
		virtual expected<void, std::string> onGamepadStickDir(uint8_t num, GamepadStick stick, InputDirType dir, bool active) noexcept { return {}; };
		virtual expected<void, std::string> onGamepadButton(uint8_t num, GamepadButton button, bool down) noexcept { return {}; };
		virtual expected<void, std::string> onGamepadConnect(uint8_t num, bool connected) noexcept { return {}; };
	};

	template<typename T>
	class DARMOK_EXPORT BX_NO_VTABLE ITypeGamepadListener : public IGamepadListener
	{
	public:
		entt::id_type getGamepadListenerType() const noexcept override
		{
			return entt::type_hash<T>::value();
		}
	};

	class DARMOK_EXPORT BX_NO_VTABLE IGamepadListenerFilter
	{
	public:
		virtual ~IGamepadListenerFilter() = default;
		virtual bool operator()(const IGamepadListener& listener) const noexcept = 0;
	};

	class GamepadImpl;

#define DARMOK_GAMEPAD_MAX 4

	class DARMOK_EXPORT Gamepad final
	{
	public:
		static const uint8_t MaxAmount;
		static const uint8_t Any;

		Gamepad() noexcept;
		~Gamepad() noexcept;
		Gamepad(const Gamepad& other) = delete;
		Gamepad(Gamepad&& other) = delete;
		
		
		[[nodiscard]] const glm::vec3& getStick(GamepadStick stick) const noexcept;
		[[nodiscard]] const GamepadSticks& getSticks() const noexcept;
		[[nodiscard]] bool getStickDir(GamepadStick stick, InputDirType dir) const noexcept;
		[[nodiscard]] bool getButton(GamepadButton button) const noexcept;
		[[nodiscard]] const GamepadButtons& getButtons() const noexcept;
		[[nodiscard]] bool isConnected() const noexcept;

		void addListener(std::unique_ptr<IGamepadListener>&& listener) noexcept;
		void addListener(IGamepadListener& listener) noexcept;
		bool removeListener(const IGamepadListener& listener) noexcept;
		size_t removeListeners(const IGamepadListenerFilter& filter) noexcept;

		[[nodiscard]] const GamepadImpl& getImpl() const noexcept;
		[[nodiscard]] GamepadImpl& getImpl() noexcept;

		[[nodiscard]] static std::optional<GamepadButton> readButton(std::string_view name) noexcept;
		[[nodiscard]] static std::string_view getButtonName(GamepadButton button) noexcept;
		[[nodiscard]] static std::optional<GamepadStick> readStick(std::string_view name) noexcept;
		[[nodiscard]] static std::string_view getStickName(GamepadStick stick) noexcept;

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

	struct DARMOK_EXPORT GamepadStickInputEvent final
	{
		GamepadStick stick = GamepadStick::Left;
		InputDirType dir = InputDirType::Left;
		uint8_t gamepad = Gamepad::Any;

		bool operator==(const GamepadStickInputEvent& other) const noexcept;
	};

	using InputEvent = std::variant<KeyboardInputEvent, MouseInputEvent, GamepadInputEvent, GamepadStickInputEvent>;
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
	using InputAxis = std::pair<InputDirs, InputDirs>;

	static bool operator==(const InputDir& a, const InputDir& b) noexcept;

	using Gamepads = std::array<Gamepad, DARMOK_GAMEPAD_MAX>;

	class InputImpl;

	class DARMOK_EXPORT BX_NO_VTABLE IInputEventListener
	{
	public:
		virtual ~IInputEventListener() = default;
		virtual entt::id_type getInputEventListenerType() const noexcept { return 0; };
		virtual expected<void, std::string> onInputEvent(const std::string& tag) noexcept = 0;
	};

	template<typename T>
	class DARMOK_EXPORT BX_NO_VTABLE ITypeInputEventListener : public IInputEventListener
	{
	public:
		entt::id_type getInputEventListenerType() const noexcept override
		{
			return entt::type_hash<T>::value();
		}
	};

	class DARMOK_EXPORT BX_NO_VTABLE IInputEventListenerFilter
	{
	public:
		virtual ~IInputEventListenerFilter() = default;
		virtual bool operator()(const std::string& tag, const IInputEventListener& listener) const noexcept = 0;
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
		Input(const Input&) = delete;
		Input(Input&&) = delete;
		Input& operator=(const Input&) = delete;
		Input& operator=(Input&&) = delete;

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
		float getAxis(const InputDirs& negative, const InputDirs& positive, const Sensitivity& sensitivity = {}) const noexcept;
		float getAxis(const InputAxis& axis, const Sensitivity& sensitivity = {}) const noexcept;

		template<glm::length_t L, typename T = float, glm::qualifier Q = glm::defaultp, size_t L2 = L>
		void getAxis(glm::vec<L, T, Q>& v, const std::array<InputAxis, L2>& axis, const Sensitivity& sensitivity = {}) const noexcept
		{
			for (glm::length_t i = 0; i < L; ++i)
			{
				v[i] = getAxis(axis[i], sensitivity);
			}
		}
		
		void addListener(const std::string& tag, const InputEvents& evs, std::unique_ptr<IInputEventListener>&& listener) noexcept;
		void addListener(const std::string& tag, const InputEvents& evs, IInputEventListener& listener) noexcept;
		bool removeListener(const std::string& tag, const IInputEventListener& listener) noexcept;
		bool removeListener(const IInputEventListener& listener) noexcept;
		size_t removeListeners(const IInputEventListenerFilter& filter) noexcept;

		[[nodiscard]] static bool matchesEvent(const InputEvent& condition, const InputEvent& real) noexcept;
		[[nodiscard]] static expected<InputEvent, std::string> readEvent(std::string_view name) noexcept;
		[[nodiscard]] static expected<InputDir, std::string> readDir(std::string_view name) noexcept;
		[[nodiscard]] static std::optional<InputDirType> readDirType(std::string_view name) noexcept;
		[[nodiscard]] static std::string_view getDirTypeName(InputDirType type) noexcept;

	private:
		std::unique_ptr<InputImpl> _impl;
	};
}