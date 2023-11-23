#pragma once

#include <darmok/entry.hpp>
#include <darmok/utils.hpp>
#include <functional>

namespace darmok
{

	struct InputBinding
	{
		void set(Key key, uint8_t modifiers, uint8_t flags, std::function<void()> fn)
		{
			key = key;
			modifiers = modifiers;
			flags = flags;
			fn = fn;
		}

		void end()
		{
			key = Key::None;
			modifiers = to_underlying(Modifier::None);
			flags = 0;
			fn = nullptr;
		}

		Key key;
		uint8_t modifiers;
		uint8_t flags;
		std::function<void()> fn;
		std::string name;
	};

#define INPUT_BINDING_END { Key::None, to_underlying(Modifier::None), 0, NULL, NULL }

	///
	void inputInit();

	///
	void inputShutdown();

	///
	void inputAddBindings(const std::string& name, const InputBinding* bindings);

	///
	void inputRemoveBindings(const std::string& name);

	///
	void inputProcess();

	///
	void inputSetKeyState(Key key, uint8_t modifiers, bool down);

	///
	bool inputGetKeyState(Key key, uint8_t& modifiers);

	///
	uint8_t inputGetModifiersState();

	/// Adds single UTF-8 encoded character into input buffer.
	void inputChar(uint8_t len, std::array<uint8_t, 4>&& data);

	/// Returns single UTF-8 encoded character from input buffer.
	const void inputGetChar(std::array<uint8_t, 4>& data);

	/// Flush internal input buffer.
	void inputCharFlush();

	///
	void inputSetMouseResolution(uint16_t width, uint16_t height);

	///
	void inputSetMousePos(int32_t x, int32_t y, int32_t z);

	///
	void inputSetMouseButtonState(MouseButton button, uint8_t state);

	///
	void inputSetMouseLock(bool lock);

	///
	void inputGetMouse(float mouse[3]);

	///
	bool inputIsMouseLocked();

	///
	void inputSetGamepadAxis(GamepadHandle handle, GamepadAxis axis, int32_t value);

	///
	int32_t inputGetGamepadAxis(GamepadHandle handle, GamepadAxis axis);
}

