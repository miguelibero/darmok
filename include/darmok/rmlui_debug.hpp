#pragma once

#ifdef _DEBUG

#include <darmok/input.hpp>
#include <darmok/app.hpp>
#include <memory>

namespace darmok
{
	struct DARMOK_EXPORT RmluiDebuggerAppComponentConfig
	{
		std::optional<InputEvent> enableEvent = KeyboardInputEvent{ KeyboardKey::F9 };
	};

    class RmluiAppComponent;
    class RmluiDebuggerAppComponentImpl;

	class DARMOK_EXPORT RmluiDebuggerAppComponent final : public IAppComponent
    {
    public:
		using Config = RmluiDebuggerAppComponentConfig;
		RmluiDebuggerAppComponent(RmluiAppComponent& comp, const Config& config = {}) noexcept;
		~RmluiDebuggerAppComponent() noexcept;

		void init(App& app) override;
		void shutdown() noexcept override;
	private:
		std::unique_ptr<RmluiDebuggerAppComponentImpl> _impl;
    };
}

#endif

