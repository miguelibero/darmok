#pragma once

#include <darmok/window.hpp>
#include <memory>

namespace darmok
{
	class WindowImpl final
	{
	public:
		WindowImpl();
		void init(const WindowHandle& handle, const WindowCreationOptions& options);
		void reset();

		void setPosition(const WindowPosition& pos);
		void setSize(const WindowSize& size);
		void setTitle(const std::string& title);
		void setFlags(uint32_t flags);
		void setDropFilePath(const std::string& filePath);
		void setSuspendPhase(WindowSuspendPhase phase);
		void setHandle(const WindowHandle& handle);
		
		const WindowPosition& getPosition() const;
		const WindowSize& getSize() const;
		const std::string& getTitle() const;
		const WindowHandle& getHandle() const;
		uint32_t getFlags() const;
		const std::string& getDropFilePath() const;
		WindowSuspendPhase getSuspendPhase() const;
		bool isRunning() const;

		void* getNativeHandle() const;
		static void* getNativeDisplayHandle();
		bgfx::NativeWindowHandleType::Enum getNativeHandleType() const;

		const bgfx::FrameBufferHandle& getFrameBuffer() const;

	private:
		WindowImpl(const WindowImpl& other) = delete;
		WindowImpl(WindowImpl&& other) = delete;

		bool resetFrameBuffer();
		void createFrameBuffer();

		WindowHandle _handle;
		std::string _dropFilePath;
		WindowSize _size;
		WindowPosition _pos;
		bool _mouseLock;
		uint32_t _flags;
		std::string _title;
		WindowSuspendPhase _suspendPhase;
		bgfx::FrameBufferHandle _frameBuffer;
	};

	class WindowContextImpl final
	{
	public:
		WindowContextImpl();
		Window& getWindow(const WindowHandle& handle = Window::DefaultHandle);
		Windows& getWindows();
	private:
		WindowContextImpl(const WindowContextImpl& other) = delete;
		WindowContextImpl(WindowContextImpl&& other) = delete;

		Windows _windows;
	};

} // namespace darmok