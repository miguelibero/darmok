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

		void shutdown() noexcept;
		void afterUpdate() noexcept;

		bool getKey(KeyboardKey key) const noexcept;
		const KeyboardKeys& getKeys() const noexcept;
		const KeyboardModifiers& getModifiers() const noexcept;
		bool hasModifier(KeyboardModifier mod) const noexcept;
		const KeyboardChars& getUpdateChars() const noexcept;

		void addListener(IKeyboardListener& listener) noexcept;
		void addListener(std::unique_ptr<IKeyboardListener>&& listener) noexcept;
		bool removeListener(const IKeyboardListener& listener) noexcept;
		size_t removeListeners(const IKeyboardListenerFilter& filter) noexcept;

		void flush() noexcept;
		void reset() noexcept;
		void setKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down) noexcept;
		void pushChar(const UtfChar& data) noexcept;

		static char keyToAscii(KeyboardKey key, bool upper = false) noexcept;
		
		static std::string_view getKeyName(KeyboardKey key) noexcept;
		static std::string_view getModifierName(KeyboardModifier mod) noexcept;

		static std::optional<KeyboardKey> readKey(std::string_view name) noexcept;
		static std::optional<KeyboardModifier> readModifier(std::string_view name) noexcept;
		static std::optional<KeyboardInputEvent> readEvent(std::string_view name) noexcept;

	private:
		UtfChar popChar() noexcept;

		KeyboardKeys _keys;
		KeyboardChars _updateChars;
		KeyboardModifiers _modifiers;
		std::array<UtfChar, 256> _chars;
		size_t _charsRead;
		size_t _charsWrite;
		OwnRefCollection<IKeyboardListener> _listeners;

		static const std::string _keyPrefix;
		static const std::string _modPrefix;
	};

#pragma endregion Keyboard

#pragma region Mouse

	class MouseImpl final
	{
	public:
		MouseImpl() noexcept;
		MouseImpl(const MouseImpl& other) = delete;
		MouseImpl(MouseImpl&& other) = delete;

		void shutdown() noexcept;
		void afterUpdate(float deltaTime) noexcept;

		bool getButton(MouseButton button) const noexcept;
		bool getActive() const noexcept;
		const glm::vec2& getPosition() const noexcept;
		const glm::vec2& getVelocity() const noexcept;
		const glm::vec2& getScroll() const noexcept;
		const MouseButtons& getButtons() const noexcept;

		void addListener(IMouseListener& listener) noexcept;
		void addListener(std::unique_ptr<IMouseListener>&& listener) noexcept;
		bool removeListener(const IMouseListener& listener) noexcept;
		size_t removeListeners(const IMouseListenerFilter& filter) noexcept;

		bool setActive(bool active) noexcept;
		bool setPosition(const glm::vec2& pos) noexcept;
		bool setScroll(const glm::vec2& scroll) noexcept;
		bool setButton(MouseButton button, bool down) noexcept;

		static std::string_view getButtonName(MouseButton button) noexcept;
		static std::optional<MouseButton> readButton(std::string_view name) noexcept;
		static std::string_view getAnalogName(MouseAnalog analog) noexcept;
		static std::optional<MouseAnalog> readAnalog(std::string_view name) noexcept;
		static std::optional<MouseInputEvent> readEvent(std::string_view name) noexcept;
		static std::optional<MouseInputDir> readDir(std::string_view name) noexcept;
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
		static const std::string _analogPrefix;
	};

#pragma endregion Mouse

#pragma region Gamepad

	class GamepadImpl final
	{
	public:
		GamepadImpl() noexcept;
		GamepadImpl(const GamepadImpl& other) = delete;
		GamepadImpl(GamepadImpl&& other) = delete;

		void shutdown() noexcept;

		const glm::vec3& getStick(GamepadStick stick) const noexcept;
		const GamepadSticks& getSticks() const noexcept;
		bool getStickDir(GamepadStick stick, InputDirType dir) const noexcept;
		bool getButton(GamepadButton button) const noexcept;
		const GamepadButtons& getButtons() const noexcept;
		bool isConnected() const noexcept;

		void addListener(IGamepadListener& listener) noexcept;
		void addListener(std::unique_ptr<IGamepadListener>&& listener) noexcept;
		bool removeListener(const IGamepadListener& listener) noexcept;
		size_t removeListeners(const IGamepadListenerFilter& filter) noexcept;

		bool setNumber(uint8_t num) noexcept;
		bool setConnected(bool value) noexcept;

		bool setStick(GamepadStick stick, const glm::vec3& value) noexcept;
		bool setButton(GamepadButton button, bool down) noexcept;

		static std::optional<GamepadButton> readButton(std::string_view name) noexcept;
		static std::string_view getButtonName(GamepadButton button) noexcept;
		static std::optional<GamepadStick> readStick(std::string_view name) noexcept;
		static std::string_view getStickName(GamepadStick stick) noexcept;

		static std::optional<uint8_t> readNum(std::string_view name) noexcept;
		static std::optional<GamepadInputEvent> readEvent(std::string_view name) noexcept;
		static std::optional<GamepadInputDir> readDir(std::string_view name) noexcept;
	private:
		uint8_t _num;
		bool _connected;
		GamepadButtons _buttons;
		GamepadSticks _sticks;
		OwnRefCollection<IGamepadListener> _listeners;
		std::unordered_map<GamepadStick, std::unordered_map<InputDirType, bool>> _stickDirs;
		static const float _stickThreshold;

		static const std::string _buttonPrefix;
		static const std::string _stickPrefix;

		void clear() noexcept;
	};

#pragma endregion Gamepad

#pragma region Input

	class InputImpl final : public ITypeKeyboardListener<InputImpl>, public ITypeMouseListener<InputImpl>, public ITypeGamepadListener<InputImpl>
	{
	public:
		InputImpl(Input& input) noexcept;
		InputImpl(const InputImpl& other) = delete;
		InputImpl(InputImpl&& other) = delete;

		void shutdown() noexcept;

		Keyboard& getKeyboard() noexcept;
		Mouse& getMouse() noexcept;
		OptionalRef<Gamepad> getGamepad(uint8_t num) noexcept;
		Gamepads& getGamepads() noexcept;
		const Keyboard& getKeyboard() const noexcept;
		const Mouse& getMouse() const noexcept;
		OptionalRef<const Gamepad> getGamepad(uint8_t num) const noexcept;
		const Gamepads& getGamepads() const noexcept;

		void addListener(const std::string& tag, const InputEvents& evs, IInputEventListener& listener) noexcept;
		void addListener(const std::string& tag, const InputEvents& evs, std::unique_ptr<IInputEventListener>&& listener) noexcept;
		bool removeListener(const std::string& tag, const IInputEventListener& listener) noexcept;
		bool removeListener(const IInputEventListener& listener) noexcept;
		size_t removeListeners(const IInputEventListenerFilter& filter) noexcept;

		bool checkEvent(const InputEvent& ev) const noexcept;
		bool checkEvents(const InputEvents& evs) const noexcept;

		using Sensitivity = InputSensitivity;
		using Dirs = InputDirs;
		using Dir = InputDir;
		float getAxis(const Dirs& neg, const Dirs& pos, const Sensitivity& sensi) const noexcept;

		void afterUpdate(float deltaTime) noexcept;

		void onKeyboardKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down) override;
		void onMouseButton(MouseButton button, bool down) override;
		void onGamepadButton(uint8_t num, GamepadButton button, bool down) override;
		void onGamepadStickDir(uint8_t num, GamepadStick stick, InputDirType dir, bool active) override;

		static std::optional<InputDirType> readDirType(std::string_view name) noexcept;
		static std::string_view getDirTypeName(InputDirType type) noexcept;
		static std::optional<InputEvent> readEvent(std::string_view name) noexcept;
		static std::optional<InputDir> readDir(std::string_view name) noexcept;
		static float getDir(const glm::vec2& vec, InputDirType dir) noexcept;

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
			std::reference_wrapper<IInputEventListener> listener;
			std::unique_ptr<IInputEventListener> listenerPointer;
		};

		std::vector<ListenerData> _listeners;

		static const std::string _dirTypePrefix;

		float getDir(const Dir& dir, const Sensitivity& sensi) const noexcept;
	};

#pragma endregion Input

}
