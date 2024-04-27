#pragma once

namespace darmok
{
	enum class WindowPhase
	{
		Unknown,
		Running,
		Suspended,
		Destroyed,
		Count,
	};

	enum class WindowMode
	{
		Normal,
		Fullscreen,
		WindowedFullscreen,
		Count,
	};

	enum class WindowCursorMode
	{
		Normal,
		Hidden,
		Disabled,
	};

}