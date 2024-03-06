#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <optional>
#include <memory>
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>


namespace darmok
{
	BGFX_HANDLE(WindowHandle);

	using WindowPosition = glm::vec<2, int32_t>;
	using WindowSize = glm::vec<2, int32_t>;

	enum class WindowSuspendPhase
	{
		None,
		WillSuspend,
		DidSuspend,
		WillResume,
		DidResume,
		Count
	};

	struct WindowFlags final
	{
		static constexpr uint32_t None = 0;
		static constexpr uint32_t AspectRatio = 1;
		static constexpr uint32_t Frame = 2;
	};

	struct WindowCreationOptions final
	{
		WindowSize size;
		std::string title;
		std::optional<WindowPosition> pos;
		uint32_t flags = WindowFlags::None;
	};

	class WindowContextImpl;
	class WindowImpl;

	class Window final
	{
	public:

		static constexpr uint16_t MaxAmount = 8;
		static constexpr WindowHandle DefaultHandle = { 0 };
		static constexpr WindowHandle InvalidHandle = { bgfx::kInvalidHandle };

		Window(const Window& other) = delete;
		Window(Window&& other) = delete;

		void destroy() noexcept;
		void setPosition(const WindowPosition& pos) noexcept;
		void setSize(const WindowSize& size) noexcept;
		void setTitle(const std::string& title) noexcept;
		void setFlags(uint32_t flags, bool enabled) noexcept;
		void toggleFullscreen() noexcept;
		void setMouseLock(bool lock) noexcept;
		void requestScreenshot(std::string_view path) noexcept;

		[[nodiscard]] void* getNativeHandle() const noexcept;
		[[nodiscard]] static void* getNativeDisplayHandle() noexcept;
		[[nodiscard]] bgfx::NativeWindowHandleType::Enum getNativeHandleType() const noexcept;

		[[nodiscard]] const WindowPosition& getPosition() const noexcept;
		[[nodiscard]] const WindowSize& getSize() const noexcept;
		[[nodiscard]] const std::string& getTitle() const noexcept;
		[[nodiscard]] const WindowHandle& getHandle() const noexcept;
		[[nodiscard]] uint32_t getFlags() const noexcept;
		[[nodiscard]] const std::string& getDropFilePath() const noexcept;
		[[nodiscard]] WindowSuspendPhase getSuspendPhase() const noexcept;
		[[nodiscard]] bool isRunning() const noexcept;
		[[nodiscard]] bool isSuspended() const noexcept;
		[[nodiscard]] const WindowImpl& getImpl() const noexcept;
		[[nodiscard]] WindowImpl& getImpl() noexcept;

	private:
		Window() noexcept;
		std::unique_ptr<WindowImpl> _impl;
		friend WindowContextImpl;
	};

	using Windows = std::array<Window, Window::MaxAmount> ;

	class WindowContext final
	{
	public:
		WindowContext(const WindowContext& other) = delete;
		WindowContext(WindowContext&& other) = delete;

		Window& createWindow(const WindowCreationOptions& options);
		Window& getWindow(const WindowHandle& handle = Window::DefaultHandle) noexcept;
		[[nodiscard]] const Window& getWindow(const WindowHandle& handle = Window::DefaultHandle) const noexcept;
		[[nodiscard]] Windows& getWindows() noexcept;
		static WindowContext& get() noexcept;

	private:
		WindowContext() noexcept;

		std::unique_ptr<WindowContextImpl> _impl;
	};

} // namespace darmok

template<> struct std::hash<darmok::WindowHandle> {
	std::size_t operator()(darmok::WindowHandle const& handle) const noexcept {
		return handle.idx;
	}
};