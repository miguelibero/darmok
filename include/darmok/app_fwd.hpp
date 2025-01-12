#pragma once

namespace darmok
{
	enum class AppRunResult
	{
		Continue,
		Exit,
		Restart,
	};

	enum class AppPhase
	{
		Setup,
		Init,
		Update,
		Shutdown
	};
}