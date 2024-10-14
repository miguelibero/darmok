#pragma once

#include <darmok/input.hpp>
#include <darmok/collection.hpp>

#include <string>
#include <array>

namespace darmok
{
#pragma region Keyboard

	class KeyboardImpl final
	{
	public:
		KeyboardImpl() noexcept;
		KeyboardImpl(const KeyboardImpl& other) = delete;
		KeyboardImpl(KeyboardImpl&& other) = delete;

		[[nodiscard]] bool getKey(KeyboardKey key) const noexcept;
		[[nodiscard]] const KeyboardKeys& getKeys() const noexcept;
		[[nodiscard]] const KeyboardModifiers& getModifiers() const noexcept;
		[[nodiscard]] bool hasModifier(KeyboardModifier mod) const noexcept;
		[[nodiscard]] const KeyboardChars& getUpdateChars() const noexcept;

		void addListener(IKeyboardListener& listener, entt::id_type type = 0) noexcept;
		void addListener(std::unique_ptr<IKeyboardListener>&& listener, entt::id_type type = 0) noexcept;
		void getListeners(std::vector<std::reference_wrapper<IKeyboardListener>>& listeners, entt::id_type type) noexcept;
		bool removeListener(const IKeyboardListener& listener) noexcept;
		size_t removeListeners(const IKeyboardListenerFilter& filter) noexcept;

		void flush() noexcept;
		void reset() noexcept;
		void setKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down) noexcept;
		void pushChar(const Utf8Char& data) noexcept;

		void afterUpdate() noexcept;

		[[nodiscard]] static char keyToAscii(KeyboardKey key, bool upper = false) noexcept;
		
		[[nodiscard]] static const std::string& getKeyName(KeyboardKey key) noexcept;
		[[nodiscard]] static const std::string& getModifierName(KeyboardModifier mod) noexcept;

		[[nodiscard]] static std::optional<KeyboardKey> readKey(std::string_view name) noexcept;
		[[nodiscard]] static std::optional<KeyboardModifier> readModifier(std::string_view name) noexcept;
		[[nodiscard]] static std::optional<KeyboardInputEvent> readEvent(std::string_view name) noexcept;

	private:
		Utf8Char popChar() noexcept;

		KeyboardKeys _keys;
		KeyboardChars _updateChars;
		KeyboardModifiers _modifiers;
		std::array<Utf8Char, 256> _chars;
		size_t _charsRead;
		size_t _charsWrite;
		OwnRefCollection<IKeyboardListener> _listeners;

		static const std::string _keyPrefix;
		static const std::array<std::string, toUnderlying(KeyboardKey::Count)> _keyNames;
		static const std::string _modPrefix;
		static const std::array<std::string, toUnderlying(KeyboardModifier::Count)> _modNames;
	};

#pragma endregion Keyboard

#pragma region Mouse

	class MouseImpl final
	{
	public:
		MouseImpl() noexcept;
		MouseImpl(const MouseImpl& other) = delete;
		MouseImpl(MouseImpl&& other) = delete;

		[[nodiscard]] bool getButton(MouseButton button) const noexcept;
		[[nodiscard]] bool getActive() const noexcept;
		[[nodiscard]] const glm::vec2& getPosition() const noexcept;
		[[nodiscard]] const glm::vec2& getVelocity() const noexcept;
		[[nodiscard]] const glm::vec2& getScroll() const noexcept;
		[[nodiscard]] const MouseButtons& getButtons() const noexcept;

		void addListener(IMouseListener& listener, entt::id_type type = 0) noexcept;
		void addListener(std::unique_ptr<IMouseListener>&& listener, entt::id_type type = 0) noexcept;
		bool removeListener(const IMouseListener& listener) noexcept;
		size_t removeListeners(const IMouseListenerFilter& filter) noexcept;

		bool setActive(bool active) noexcept;
		bool setPosition(const glm::vec2& pos) noexcept;
		bool setScroll(const glm::vec2& scroll) noexcept;
		bool setButton(MouseButton button, bool down) noexcept;

		void afterUpdate(float deltaTime) noexcept;

		[[nodiscard]] static const std::string& getButtonName(MouseButton button) noexcept;
		[[nodiscard]] static std::optional<MouseButton> readButton(std::string_view name) noexcept;
		[[nodiscard]] static const std::string& getAnalogName(MouseAnalog analog) noexcept;
		[[nodiscard]] static std::optional<MouseAnalog> readAnalog(std::string_view name) noexcept;
		[[nodiscard]] static std::optional<MouseInputEvent> readEvent(std::string_view name) noexcept;
		[[nodiscard]] static std::optional<MouseInputDir> readDir(std::string_view name) noexcept;

	private:
		glm::vec2 _position;
		glm::vec2 _lastPosition;
		float _lastPositionTimePassed;
		glm::vec2 _velocity;
		glm::vec2 _scroll;

		MouseButtons _buttons;
		bool _active;
		bool _hasBeenInactive;
		OwnRefCollection<IMouseListener> _listeners;

		static const std::string _buttonPrefix;
		static const std::array<std::string, toUnderlying(MouseButton::Count)> _buttonNames;
		static const std::string _analogPrefix;
		static const std::array<std::string, toUnderlying(MouseAnalog::Count)> _analogNames;
	};

#pragma endregion Mouse

#pragma region Gamepad

	class GamepadImpl final
	{
	public:
		GamepadImpl() noexcept;
		GamepadImpl(const GamepadImpl& other) = delete;
		GamepadImpl(GamepadImpl&& other) = delete;

		[[nodiscard]] const glm::vec3& getStick(GamepadStick stick) const noexcept;
		[[nodiscard]] const GamepadSticks& getSticks() const noexcept;
		[[nodiscard]] bool getButton(GamepadButton button) const noexcept;
		[[nodiscard]] const GamepadButtons& getButtons() const noexcept;
		[[nodiscard]] bool isConnected() const noexcept;

		void addListener(IGamepadListener& listener, entt::id_type type = 0) noexcept;
		void addListener(std::unique_ptr<IGamepadListener>&& listener, entt::id_type type = 0) noexcept;
		bool removeListener(const IGamepadListener& listener) noexcept;
		size_t removeListeners(const IGamepadListenerFilter& filter) noexcept;

		bool setNumber(uint8_t num) noexcept;
		bool setConnected(bool value) noexcept;

		bool setStick(GamepadStick stick, const glm::vec3& value) noexcept;
		bool setButton(GamepadButton button, bool down) noexcept;

		[[nodiscard]] static std::optional<GamepadButton> readButton(std::string_view name) noexcept;
		[[nodiscard]] static const std::string& getButtonName(GamepadButton button) noexcept;
		[[nodiscard]] static std::optional<GamepadStick> readStick(std::string_view name) noexcept;
		[[nodiscard]] static const std::string& getStickName(GamepadStick stick) noexcept;

		[[nodiscard]] static std::optional<uint8_t> readNum(std::string_view name) noexcept;
		[[nodiscard]] static std::optional<GamepadInputEvent> readEvent(std::string_view name) noexcept;
		[[nodiscard]] static std::optional<GamepadInputDir> readDir(std::string_view name) noexcept;
	private:
		uint8_t _num;
		bool _connected;
		GamepadButtons _buttons;
		GamepadSticks _sticks;
		OwnRefCollection<IGamepadListener> _listeners;

		static const std::string _buttonPrefix;
		static const std::array<std::string, toUnderlying(GamepadButton::Count)> _buttonNames;
		static const std::string _stickPrefix;
		static const std::array<std::string, toUnderlying(GamepadStick::Count)> _stickNames;

		void clear() noexcept;
	};

#pragma endregion Gamepad

#pragma region Input

	class InputImpl final : public IKeyboardListener, public IMouseListener, public IGamepadListener
	{
	public:
		InputImpl(Input& input) noexcept;
		InputImpl(const InputImpl& other) = delete;
		InputImpl(InputImpl&& other) = delete;

		Keyboard& getKeyboard() noexcept;
		Mouse& getMouse() noexcept;
		OptionalRef<Gamepad> getGamepad(uint8_t num) noexcept;
		Gamepads& getGamepads() noexcept;
		const Keyboard& getKeyboard() const noexcept;
		const Mouse& getMouse() const noexcept;
		OptionalRef<const Gamepad> getGamepad(uint8_t num) const noexcept;
		const Gamepads& getGamepads() const noexcept;

		void addListener(const std::string& tag, const InputEvents& evs, IInputEventListener& listener, entt::id_type type = 0) noexcept;
		void addListener(const std::string& tag, const InputEvents& evs, std::unique_ptr<IInputEventListener>&& listener, entt::id_type type = 0) noexcept;
		bool removeListener(const std::string& tag, const IInputEventListener& listener) noexcept;
		bool removeListener(const IInputEventListener& listener) noexcept;
		size_t removeListeners(const IInputEventListenerFilter& filter) noexcept;

		bool checkEvent(const InputEvent& ev) const noexcept;
		bool checkEvents(const InputEvents& evs) const noexcept;

		using Sensitivity = InputSensitivity;
		using Dirs = InputDirs;
		using Dir = InputDir;
		float getAxis(const Dirs& pos, const Dirs& neg, const Sensitivity& sensi) const noexcept;

		void afterUpdate(float deltaTime) noexcept;

		void onKeyboardKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down) override;
		void onMouseButton(MouseButton button, bool down) override;
		void onGamepadButton(uint8_t num, GamepadButton button, bool down) override;

		static std::optional<InputDirType> readDirType(std::string_view name) noexcept;
		static const std::string& getDirTypeName(InputDirType type) noexcept;
		static std::optional<InputEvent> readEvent(std::string_view name) noexcept;
		static std::optional<InputDir> readDir(std::string_view name) noexcept;

	private:
		Input& _input;
		Keyboard _keyboard;
		Mouse _mouse;
		Gamepads _gamepads;
		static const std::string _keyboardPrefix;
		static const std::string _mousePrefix;
		static const std::string _gamepadPrefix;
		static const float _mouseVelocityDirFactor;
		static const float _mouseScrollDirFactor;

		struct ListenerData final
		{
			std::string tag;
			InputEvents events;
			entt::id_type type;
			std::reference_wrapper<IInputEventListener> listener;
			std::unique_ptr<IInputEventListener> listenerPointer;
		};

		std::vector<ListenerData> _listeners;

		static const std::string _dirTypePrefix;
		static const std::array<std::string, toUnderlying(InputDirType::Count)> _dirTypeNames;

		float getDir(const Dir& dir, const Sensitivity& sensi) const noexcept;
		static float getDir(const glm::vec2& vec, InputDirType dir) noexcept;
	};

#pragma endregion Input

}
