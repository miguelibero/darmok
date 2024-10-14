#pragma once

#include "lua.hpp"
#include "utils.hpp"

#include <darmok/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/input.hpp>

#include <optional>
#include <vector>
#include <array>

namespace darmok
{
	class LuaKeyboardListener final : public IKeyboardListener
	{
	public:
		LuaKeyboardListener(const sol::table& table) noexcept;
		sol::object getReal() const noexcept;

		entt::id_type getType() const noexcept override;
		void onKeyboardKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down) override;
		void onKeyboardChar(const Utf8Char& chr) override;

	private:
		sol::main_table _table;
		static const LuaTableDelegateDefinition _keyDelegate;
		static const LuaTableDelegateDefinition _charDelegate;
	};

	class LuaKeyboardListenerFilter final : public IKeyboardListenerFilter
	{
	public:
		LuaKeyboardListenerFilter(const sol::table& table) noexcept;
		bool operator()(const IKeyboardListener& listener) const noexcept override;
	private:
		sol::main_table _table;
		entt::id_type _type;
	};

	class LuaKeyboard final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;

		static std::optional<KeyboardKey> readKey(const sol::object& val) noexcept;
		static std::optional<KeyboardModifier> readModifier(const sol::object& val) noexcept;
		static std::optional<KeyboardInputEvent> readEvent(const sol::object& val) noexcept;

	private:
		static void addListener(Keyboard& kb, const sol::table& table) noexcept;
		static bool removeListener(Keyboard& kb, const sol::table& table) noexcept;

		static std::string getUpdateChars(const Keyboard& kb) noexcept;
	};

	class LuaMouseListener final : public IMouseListener
	{
	public:
		LuaMouseListener(const sol::table& table) noexcept;
		sol::object getReal() const noexcept;

		entt::id_type getType() const noexcept override;
		void onMousePositionChange(const glm::vec2& delta, const glm::vec2& absolute) override;
		void onMouseScrollChange(const glm::vec2& delta, const glm::vec2& absolute) override;
		void onMouseButton(MouseButton button, bool down) override;

	private:
		sol::main_table _table;
		static const LuaTableDelegateDefinition _posDelegate;
		static const LuaTableDelegateDefinition _scrollDelegate;
		static const LuaTableDelegateDefinition _buttonDelegate;
	};

	class LuaMouseListenerFilter final : public IMouseListenerFilter
	{
	public:
		LuaMouseListenerFilter(const sol::table& table) noexcept;
		bool operator()(const IMouseListener& listener) const noexcept override;
	private:
		sol::main_table _table;
		entt::id_type _type;
	};

	class LuaMouse final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;

		static std::optional<MouseButton> readButton(const sol::object& val) noexcept;
		static std::optional<MouseInputEvent> readEvent(const sol::object& val) noexcept;
		static std::optional<MouseInputDir> readDir(const sol::object& val) noexcept;
		static std::optional<MouseAnalog> readAnalog(const sol::object& val) noexcept;

	private:
		static bool getLeftButton(const Mouse& mouse) noexcept;
		static bool getMiddleButton(const Mouse& mouse) noexcept;
		static bool getRightButton(const Mouse& mouse) noexcept;

		static void addListener(Mouse& mouse, const sol::table& table) noexcept;
		static bool removeListener(Mouse& mouse, const sol::table& table) noexcept;
	};

	class LuaGamepadListener final : public IGamepadListener
	{
	public:
		LuaGamepadListener(const sol::table& table) noexcept;
		sol::object getReal() const noexcept;

		entt::id_type getType() const noexcept override;
		void onGamepadStickChange(uint8_t num, GamepadStick stick, const glm::vec3& delta, const glm::vec3& absolute) override;
		void onGamepadButton(uint8_t num, GamepadButton button, bool down) override;
		void onGamepadConnect(uint8_t num, bool connected) override;

	private:
		sol::main_table _table;
		static const LuaTableDelegateDefinition _stickDelegate;
		static const LuaTableDelegateDefinition _buttonDelegate;
		static const LuaTableDelegateDefinition _connectDelegate;
	};

	class LuaGamepadListenerFilter final : public IGamepadListenerFilter
	{
	public:
		LuaGamepadListenerFilter(const sol::table& table) noexcept;
		bool operator()(const IGamepadListener& listener) const noexcept override;
	private:
		sol::main_table _table;
		entt::id_type _type;
	};

	class LuaGamepad final : IGamepadListener
	{
	public:
		static void bind(sol::state_view& lua) noexcept;

		static std::optional<GamepadInputEvent> readEvent(const sol::object& val) noexcept;
		static std::optional<GamepadButton> readButton(const sol::object& val) noexcept;
		static std::optional<GamepadStick> readStick(const sol::object& val) noexcept;
		static std::optional<GamepadInputDir> readDir(const sol::object& val) noexcept;

	private:
		static void addListener(Gamepad& gamepad, const sol::table& table) noexcept;
		static bool removeListener(Gamepad& gamepad, const sol::table& table) noexcept;

		static const glm::vec3& getLeftStick(const Gamepad& gamepad) noexcept;
		static const glm::vec3& getRightStick(const Gamepad& gamepad) noexcept;
	};

	class LuaInputEventListener final : public IInputEventListener
	{
	public:
		LuaInputEventListener(const sol::object& dlg) noexcept;
		const LuaDelegate& getDelegate() const noexcept;

		entt::id_type getType() const noexcept override;
		void onInputEvent(const std::string& tag) override;
	private:
		LuaDelegate _delegate;
	};

	class LuaInputEventListenerFilter final : public IInputEventListenerFilter
	{
	public:
		LuaInputEventListenerFilter(const sol::object& obj, std::optional<std::string> tag = std::nullopt) noexcept;
		bool operator()(const std::string& tag, const IInputEventListener& listener) const noexcept override;
	private:
		sol::main_object _obj;
		entt::id_type _type;
		std::optional<std::string> _tag;
	};

	class LuaInput final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;

		static std::optional<InputDirType> readDirType(const sol::object& val) noexcept;

	private:
		static void addListener(Input& input, const std::string& tag, const sol::object& ev, const sol::object& listener);
		static bool removeListener1(Input& input, const std::string& tag, const sol::object& listener) noexcept;
		static bool removeListener2(Input& input, const sol::object& listener) noexcept;
		static OptionalRef<Gamepad>::std_t getGamepad(Input& input, uint8_t num = 0) noexcept;

		static bool checkEvent(const Input& input, const sol::object& ev) noexcept;
		static bool checkEvents(const Input& input, const sol::table& evs) noexcept;
		static float getAxis(const Input& input, const sol::object& positive, const sol::object& negative) noexcept;

		static std::optional<InputEvent> readEvent(const sol::object& val) noexcept;
		static InputEvents readEvents(const sol::object& val) noexcept;
		static std::optional<InputDir> readDir(const sol::object& val) noexcept;
		static InputDirs readDirs(const sol::object& val) noexcept;
	};
}