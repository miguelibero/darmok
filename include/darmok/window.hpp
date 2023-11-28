/*
 * based on https://github.com/bkaradzic/bgfx/blob/master/examples/common/entry/entry.h
 */

#pragma once

#include <bgfx/bgfx.h>
#include <string>
#include <darmok/utils.hpp>

namespace darmok
{
	class WindowCreatedEvent;
	class WindowSizeChangedEvent;
	class WindowPositionChangedEvent;
	class WindowTitleChangedEvent;
	class WindowDestroyedEvent;
	class WindowSuspendedEvent;
	class FileDroppedEvent;

	/// 
	struct WindowHandle final
	{
		uint16_t idx;

		bool operator==(const WindowHandle& other) const
		{
			return idx == other.idx;
		}

		bool operator<(const WindowHandle& other) const
		{
			return idx < other.idx;
		}
	};

	/// 
	struct WindowPosition final
	{
		int32_t x;
		int32_t y;

		WindowPosition(int32_t vx = 0, int32_t vy = 0)
			: x(vx)
			, y(vy)
		{
		}

		bool operator==(const WindowPosition& other) const
		{
			return x == other.x && y == other.y;
		}
	};

	/// 
	struct WindowSize final
	{
		uint32_t width;
		uint32_t height;

		WindowSize(int32_t vwidth = 0, int32_t vheight = 0)
			: width(vwidth)
			, height(vheight)
		{
		}

		bool operator==(const WindowSize& other) const
		{
			return width == other.width && height == other.height;
		}
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
		bool setPos;
		WindowPosition pos;
		uint32_t flags = WindowFlags::None;
	};

	class Window final
	{
	public:
		///
		static constexpr WindowHandle DefaultHandle = { 0 };
		static constexpr WindowHandle InvalidHandle = { UINT16_MAX };
		static constexpr size_t MaxWindows = 8;

		Window(const WindowHandle& handle = InvalidHandle);

		/// 
		static Window& create(const WindowCreationOptions& options);

		/// 
		static Window& get(const WindowHandle& handle = DefaultHandle);

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

	private:


		WindowHandle _handle;
		std::string _dropFilePath;
		WindowSize _size;
		WindowPosition _pos;
		bool _mouseLock;
		uint32_t _flags;
		std::string _title;
		WindowSuspendPhase _suspendPhase;

		friend WindowCreatedEvent;
		friend WindowSizeChangedEvent;
		friend WindowPositionChangedEvent;
		friend WindowTitleChangedEvent;
		friend WindowDestroyedEvent;
		friend WindowSuspendedEvent;
		friend FileDroppedEvent;
	};

} // namespace darmok


template<> struct std::hash<darmok::WindowHandle> {
	std::size_t operator()(darmok::WindowHandle const& handle) const noexcept {
		return handle.idx;
	}
};