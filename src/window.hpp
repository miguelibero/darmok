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
		
		const WindowPosition& getPosition() const;
		const WindowSize& getSize() const;
		const std::string& getTitle() const;
		const WindowHandle& getHandle() const;
		uint32_t getFlags() const;
		const std::string& getDropFilePath() const;
		WindowSuspendPhase getSuspendPhase() const;
		bool isRunning() const;

	private:
		WindowImpl(const WindowImpl& other) = delete;
		WindowImpl(WindowImpl&& other) = delete;

		WindowHandle _handle;
		std::string _dropFilePath;
		WindowSize _size;
		WindowPosition _pos;
		bool _mouseLock;
		uint32_t _flags;
		std::string _title;
		WindowSuspendPhase _suspendPhase;
	};

	class ContextImpl final
	{
	public:
		ContextImpl();
		Window& getWindow(const WindowHandle& handle = Window::DefaultHandle);
		Windows& getWindows();
	private:
		ContextImpl(const ContextImpl& other) = delete;
		ContextImpl(ContextImpl&& other) = delete;

		Windows _windows;
	};

} // namespace darmok