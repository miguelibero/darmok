#pragma once

#include <darmok/window.hpp>
#include <memory>

namespace darmok
{
	class WindowImpl final
	{
	public:
		WindowImpl() noexcept;
		WindowImpl(const WindowImpl& other) = delete;
		WindowImpl(WindowImpl&& other) = delete;

		void init(const WindowHandle& handle, const WindowCreationOptions& options);
		void reset();
		void requestScreenshot(std::string_view path) noexcept;

		void setPosition(const WindowPosition& pos) noexcept;
		void setSize(const WindowSize& size) noexcept;
		void setTitle(const std::string& title) noexcept;
		void setFlags(uint32_t flags) noexcept;
		void setDropFilePath(const std::string& filePath) noexcept;
		void setHandle(const WindowHandle& handle) noexcept;
		void onSuspendPhase(WindowSuspendPhase phase) noexcept;
		
		[[nodiscard]] const WindowPosition& getPosition() const noexcept;
		[[nodiscard]] const WindowSize& getSize() const noexcept;
		[[nodiscard]] const std::string& getTitle() const noexcept;
		[[nodiscard]] const WindowHandle& getHandle() const noexcept;
		[[nodiscard]] uint32_t getFlags() const noexcept;
		[[nodiscard]] const std::string& getDropFilePath() const noexcept;
		[[nodiscard]] bool isSuspended() const noexcept;
		[[nodiscard]] bool isRunning() const noexcept;

		[[nodiscard]] void* getNativeHandle() const noexcept;
		[[nodiscard]] static void* getNativeDisplayHandle() noexcept;
		[[nodiscard]] bgfx::NativeWindowHandleType::Enum getNativeHandleType() const noexcept;
		[[nodiscard]] const bgfx::FrameBufferHandle& getFrameBuffer() const noexcept;

	private:
		bool resetFrameBuffer();
		void createFrameBuffer();

		WindowHandle _handle;
		std::string _dropFilePath;
		WindowSize _size;
		WindowPosition _pos;
		bool _mouseLock;
		uint32_t _flags;
		std::string _title;
		bool _suspended;
		bgfx::FrameBufferHandle _frameBuffer;
	};

	class WindowContextImpl final
	{
	public:
		WindowContextImpl() noexcept;
		WindowContextImpl(const WindowContextImpl& other) = delete;
		WindowContextImpl(WindowContextImpl&& other) = delete;

		Window& getWindow(const WindowHandle& handle = Window::DefaultHandle) noexcept;
		Windows& getWindows() noexcept;
	private:
		Windows _windows;
	};

} // namespace darmok