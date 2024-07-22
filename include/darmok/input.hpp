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
#include <nlohmann/json.hpp>
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

		void addListener(IKeyboardListener& listener) noexcept;
		bool removeListener(IKeyboardListener& listener) noexcept;

		[[nodiscard]] const KeyboardImpl& getImpl() const noexcept;
		[[nodiscard]] KeyboardImpl& getImpl() noexcept;

		[[nodiscard]] static char keyToAscii(KeyboardKey key, bool upper) noexcept;
		[[nodiscard]] static const std::string& getKeyName(KeyboardKey key) noexcept;
		[[nodiscard]] static const std::string& getModifierName(KeyboardModifier key) noexcept;
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

		[[nodiscard]] static std::optional<MouseButton> readButton(std::string_view name) noexcept;
		[[nodiscard]] static std::optional<MouseInputType> readInputType(std::string_view name) noexcept;
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

		[[nodiscard]] static std::optional<GamepadButton> readButton(std::string_view name) noexcept;
		[[nodiscard]] static std::optional<GamepadStick> readStick(std::string_view name) noexcept;

	private:
		std::unique_ptr<GamepadImpl> _impl;
	};

	class Input;

	struct DARMOK_EXPORT BX_NO_VTABLE IInputEvent
	{
		virtual ~IInputEvent() = default;
		virtual bool check(const Input& input) const = 0;
		virtual std::unique_ptr<IInputEvent> copy() const noexcept = 0;
		static std::unique_ptr<IInputEvent> read(const nlohmann::json& json) noexcept;
	};

	struct DARMOK_EXPORT KeyboardInputEvent final : public IInputEvent
	{
		KeyboardKey key;
		KeyboardModifiers modifiers;

		KeyboardInputEvent(KeyboardKey key, const KeyboardModifiers& mods = {}) noexcept;
		bool check(const Input& input) const noexcept override;
		std::unique_ptr<IInputEvent> copy() const noexcept override;
		static std::unique_ptr<KeyboardInputEvent> read(const nlohmann::json& json) noexcept;
	};

	struct DARMOK_EXPORT MouseInputEvent final : public IInputEvent
	{
		MouseButton button;

		MouseInputEvent(MouseButton button) noexcept;
		bool check(const Input& input) const noexcept override;
		std::unique_ptr<IInputEvent> copy() const noexcept override;
		static std::unique_ptr<MouseInputEvent> read(const nlohmann::json& json) noexcept;
	};

	struct DARMOK_EXPORT GamepadInputEvent final : public IInputEvent
	{
		GamepadButton button;
		uint8_t gamepad;

		GamepadInputEvent(GamepadButton button, uint8_t gamepad = Gamepad::Any) noexcept;
		bool check(const Input& input) const noexcept override;
		std::unique_ptr<IInputEvent> copy() const noexcept override;
		static std::unique_ptr<GamepadInputEvent> read(const nlohmann::json& json) noexcept;
	};

	struct DARMOK_EXPORT CombinedInputEvent final : public IInputEvent
	{
		using Events = std::vector<std::unique_ptr<IInputEvent>>;
		Events events;

		CombinedInputEvent(Events&& events) noexcept;
		bool check(const Input& input) const noexcept override;
		std::unique_ptr<IInputEvent> copy() const noexcept override;
		static std::unique_ptr<CombinedInputEvent> read(const nlohmann::json& json) noexcept;
	};

	struct DARMOK_EXPORT BX_NO_VTABLE IInputAxis
	{
		virtual float get(const Input& input) const = 0;
		virtual std::unique_ptr<IInputAxis> copy() const = 0;
		static std::unique_ptr<IInputAxis> read(const nlohmann::json& json) noexcept;
		static const float getValue(const glm::vec2& v, InputAxisType type) noexcept;
	};

	struct DARMOK_EXPORT MouseInputAxis final : public IInputAxis
	{
		MouseInputType input;
		InputAxisType type;

		MouseInputAxis(MouseInputType input = MouseInputType::Position, InputAxisType type = InputAxisType::Horizontal) noexcept;
		float get(const Input& input) const noexcept override;
		std::unique_ptr<IInputAxis> copy() const noexcept override;
		static std::optional<MouseInputAxis> read(const nlohmann::json& json) noexcept;
	};

	struct DARMOK_EXPORT GamepadInputAxis final : public IInputAxis
	{
		GamepadStick stick;
		InputAxisType type;
		uint8_t gamepad;

		GamepadInputAxis(GamepadStick stick = GamepadStick::Left, InputAxisType type = InputAxisType::Horizontal, uint8_t gamepad = Gamepad::Any) noexcept;
		float get(const Input& input) const noexcept override;
		std::unique_ptr<IInputAxis> copy() const noexcept override;
		static std::optional<GamepadInputAxis> read(const nlohmann::json& json) noexcept;
	};

	struct DARMOK_EXPORT ActionInputAxis final : public IInputAxis
	{
		std::unique_ptr<IInputEvent> positive;
		std::unique_ptr<IInputEvent> negative;
		float factor;

		ActionInputAxis(std::unique_ptr<IInputEvent>&& positive, std::unique_ptr<IInputEvent>&& negative, float factor = 1.F) noexcept;
		float get(const Input& input) const noexcept override;
		std::unique_ptr<IInputAxis> copy() const noexcept override;
		static std::optional<ActionInputAxis> read(const nlohmann::json& json) noexcept;
	};

	struct DARMOK_EXPORT CombinedInputAxis final : public IInputAxis
	{
		using Axes = std::vector<std::unique_ptr<IInputAxis>>;
		Axes axes;

		CombinedInputAxis(const Axes& axes) noexcept;
		float get(const Input& input) const noexcept override;
		std::unique_ptr<IInputAxis> copy() const noexcept override;
		static std::optional<ActionInputAxis> read(const nlohmann::json& json) noexcept;
	};

	using Gamepads = std::array<Gamepad, Gamepad::MaxAmount>;

	class InputImpl;

	class DARMOK_EXPORT BX_NO_VTABLE IInputEventListener
	{
	public:
		virtual ~IInputEventListener() = default;
		virtual void onInputEvent(const IInputEvent& ev) = 0;
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
		[[nodiscard]] OptionalRef<Gamepad> getGamepad(uint8_t num = 0) noexcept;
		[[nodiscard]] Gamepads& getGamepads() noexcept;

		[[nodiscard]] const Keyboard& getKeyboard() const noexcept;
		[[nodiscard]] const Mouse& getMouse() const noexcept;
		[[nodiscard]] OptionalRef<const Gamepad> getGamepad(uint8_t num = 0) const noexcept;
		[[nodiscard]] const Gamepads& getGamepads() const noexcept;
		
		[[nodiscard]] const InputImpl& getImpl() const noexcept;
		[[nodiscard]] InputImpl& getImpl() noexcept;

		void addListener(const IInputEvent& ev, IInputEventListener& listener) noexcept;
		bool removeListener(IInputEventListener& listener) noexcept;

		[[nodiscard]] static std::optional<InputAxisType> readAxisType(std::string_view name) noexcept;

	private:
		std::unique_ptr<InputImpl> _impl;
	};
}