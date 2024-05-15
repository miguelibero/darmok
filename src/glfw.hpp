#pragma once

#include "platform.hpp"
#include <darmok/window_fwd.hpp>
#include <darmok/app_fwd.hpp>
#include <bx/thread.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#if GLFW_VERSION_MINOR < 2
#	error "GLFW 3.2 or later is required"
#endif // GLFW_VERSION_MINOR < 2

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#	if DARMOK_PLATFORM_SUPPORT_WAYLAND
#		include <wayland-egl.h>
#		define GLFW_EXPOSE_NATIVE_WAYLAND
#	endif
#		define GLFW_EXPOSE_NATIVE_X11
#		define GLFW_EXPOSE_NATIVE_GLX
#elif BX_PLATFORM_OSX
#	define GLFW_EXPOSE_NATIVE_COCOA
#	define GLFW_EXPOSE_NATIVE_NSGL
#elif BX_PLATFORM_WINDOWS
#	define GLFW_EXPOSE_NATIVE_WIN32
#	define GLFW_EXPOSE_NATIVE_WGL
#endif //

namespace darmok
{
	struct MainThreadEntry
	{
		std::vector<std::string> args;
		RunAppCallback callback;
		bool finished = false;

		static int32_t threadFunc(bx::Thread* thread, void* userData);
	};

#pragma region PlatformImpl definition

	class PlatformEvent;
	class PlatformCmd;

	class PlatformImpl final
	{
	public:
		PlatformImpl() noexcept;

		int run(const std::vector<std::string>& args, RunAppCallback callback);
		
		void pushCmd(std::unique_ptr<PlatformCmd>&& cmd) noexcept;
		std::unique_ptr<PlatformEvent> pollEvent() noexcept;

		template<typename T, typename... A>
		T& pushCmd(A&&... args) noexcept
		{
			auto ptr = new T(std::forward<A>(args)...);
			pushCmd(std::unique_ptr<PlatformCmd>(ptr));
			return *ptr;
		}

		GLFWwindow* getGlfwWindow() const noexcept;
		PlatformEventQueue& getEvents() noexcept;
		
		[[nodiscard]] static void* getWindowHandle(GLFWwindow* window) noexcept;
		static void destroyWindow(GLFWwindow* window) noexcept;
		[[nodiscard]] static GLFWwindow* createWindow(const glm::uvec2& size, const char* title) noexcept;

	private:
		GLFWwindow* _window;
		PlatformEventQueue _events;
		MainThreadEntry _mte;
		bx::Thread _thread;
		std::queue<std::unique_ptr<PlatformCmd>> _cmds;
		glm::uvec2 _windowSize;
		glm::uvec2 _framebufferSize;

		glm::vec2 normalizeScreenPoint(double x, double y) noexcept;

		static uint8_t translateKeyModifiers(int mods) noexcept;

		using KeyMap = std::array<KeyboardKey, GLFW_KEY_LAST + 1>;
		static KeyMap createKeyMap() noexcept;

		static KeyboardKey translateKey(int key) noexcept;
		static MouseButton translateMouseButton(int button) noexcept;

		struct GamepadAxisConfig final
		{
			GamepadStick stick;
			size_t index;
			bool reverse;
		};

		static std::optional<GamepadAxisConfig> translateGamepadAxis(int axis) noexcept;
		static GamepadButton translateGamepadButton(int button) noexcept;

		void updateGamepads() noexcept;
		void updateGamepad(uint8_t num) noexcept;

		void joystickCallback(int jid, int action) noexcept;
		void errorCallback(int error, const char* description);
		void keyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods) noexcept;
		void charCallback(GLFWwindow* window, uint32_t scancode) noexcept;
		void scrollCallback(GLFWwindow* window, double dx, double dy) noexcept;
		void cursorPosCallback(GLFWwindow* window, double x, double y) noexcept;
		void cursorEnterCallback(GLFWwindow* window, int entered) noexcept;
		void mouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods) noexcept;
		void windowSizeCallback(GLFWwindow* window, int32_t width, int32_t height) noexcept;
		void framebufferSizeCallback(GLFWwindow* window, int32_t width, int32_t height) noexcept;

		static void staticJoystickCallback(int jid, int action) noexcept;
		static void staticErrorCallback(int error, const char* description) noexcept;
		static void staticKeyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods) noexcept;
		static void staticCharCallback(GLFWwindow* window, uint32_t scancode) noexcept;
		static void staticScrollCallback(GLFWwindow* window, double dx, double dy) noexcept;
		static void staticCursorPosCallback(GLFWwindow* window, double x, double y) noexcept;
		static void staticCursorEnterCallback(GLFWwindow* window, int entered) noexcept;
		static void staticMouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods) noexcept;
		static void staticWindowSizeCallback(GLFWwindow* window, int32_t width, int32_t height) noexcept;
		static void staticFramebufferSizeCallback(GLFWwindow* window, int32_t width, int32_t height) noexcept;
	};

#pragma endregion PlatformImpl definition

#pragma region PlatformCmds

	class BX_NO_VTABLE PlatformCmd
	{
	public:
		enum Type
		{
			CreateWindow,
			DestroyWindow,
			ChangeWindowMode,
			ChangeWindowCursorMode,
		};

		PlatformCmd(Type type) noexcept;
		static void process(PlatformCmd& cmd, PlatformImpl& plat) noexcept;

	private:
		Type  _type;
	};

	class DestroyWindowCmd final : public PlatformCmd
	{
	public:
		DestroyWindowCmd() noexcept;
		void process(GLFWwindow* glfw) noexcept;
	};

	class ChangeWindowModeCmd final : public PlatformCmd
	{
	public:
		ChangeWindowModeCmd(WindowMode mode) noexcept;
		void process(PlatformEventQueue& events, GLFWwindow* glfw) noexcept;
		
	private:
		WindowMode _mode;
	};

	class ChangeWindowCursorModeCmd final : public PlatformCmd
	{
	public:
		ChangeWindowCursorModeCmd(WindowCursorMode value) noexcept;
		void process(PlatformEventQueue& events, GLFWwindow* glfw) noexcept;
	private:
		WindowCursorMode _value;
	};

#pragma endregion PlatformCmds
}