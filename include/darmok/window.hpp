#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <optional>
#include <memory>
#include <bgfx/bgfx.h>


namespace darmok
{
	/// 
	struct WindowHandle final
	{
		typedef uint16_t idx_t;
		idx_t idx;

		bool operator==(const WindowHandle& other) const;
		bool operator<(const WindowHandle& other) const;
		bool isValid() const;
	};

	/// 
	struct WindowPosition final
	{
		int32_t x;
		int32_t y;

		WindowPosition(int32_t x = 0, int32_t y = 0);
		bool operator==(const WindowPosition& other) const;
	};

	/// 
	struct WindowSize final
	{
		uint32_t width;
		uint32_t height;

		WindowSize(int32_t width = 0, int32_t height = 0);
		bool operator==(const WindowSize& other) const;
	};

	///
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

	///
	struct WindowCreationOptions final
	{
		WindowSize size;
		std::string title;
		std::optional<WindowPosition> pos;
		uint32_t flags = WindowFlags::None;
	};

	class ContextImpl;
	class WindowImpl;

	class Window final
	{
	public:

		static constexpr WindowHandle::idx_t MaxAmount = 8;
		static constexpr WindowHandle DefaultHandle = { 0 };
		static constexpr WindowHandle InvalidHandle = { UINT16_MAX };

		/// 
		void destroy();

		/// 
		void setPosition(const WindowPosition& pos);

		///
		void setSize(const WindowSize& size);

		///
		void setTitle(const std::string& title);

		///
		void setFlags(uint32_t flags, bool enabled);

		///
		void toggleFullscreen();

		///
		void setMouseLock(bool lock);

		///
		void* getNativeHandle() const;

		/// 
		static void* getNativeDisplayHandle();

		/// 
		bgfx::NativeWindowHandleType::Enum getNativeHandleType() const;

		///
		const WindowPosition& getPosition() const;

		///
		const WindowSize& getSize() const;

		/// 
		const std::string& getTitle() const;

		/// 
		const WindowHandle& getHandle() const;

		///
		uint32_t getFlags() const;

		/// 
		const std::string& getDropFilePath() const;

		/// 
		WindowSuspendPhase getSuspendPhase() const;

		/// 
		const WindowImpl& getImpl() const;

		/// 
		WindowImpl& getImpl();

	private:
		Window();
		Window(const Window& other) = delete;
		Window(Window&& other) = delete;

		std::unique_ptr<WindowImpl> _impl;

		friend ContextImpl;
	};

	typedef std::array<Window, Window::MaxAmount> Windows;

	class Context final
	{
	public:
		/// 
		Window& createWindow(const WindowCreationOptions& options);

		/// 
		Window& getWindow(const WindowHandle& handle = Window::DefaultHandle);

		/// 
		const Window& getWindow(const WindowHandle& handle = Window::DefaultHandle) const;

		/// 
		Windows& getWindows();

		/// 
		static Context& get();

	private:
		Context();
		Context(const Context& other) = delete;
		Context(Context&& other) = delete;

		std::unique_ptr<ContextImpl> _impl;
	};

} // namespace darmok


template<> struct std::hash<darmok::WindowHandle> {
	std::size_t operator()(darmok::WindowHandle const& handle) const noexcept {
		return handle.idx;
	}
};