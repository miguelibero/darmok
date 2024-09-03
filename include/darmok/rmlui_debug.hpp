#pragma once

#ifdef _DEBUG

#include <darmok/input.hpp>
#include <darmok/app.hpp>
#include <memory>

namespace darmok
{
	struct DARMOK_EXPORT RmluiDebuggerComponentConfig
	{
		std::optional<InputEvent> enableEvent = KeyboardInputEvent{ KeyboardKey::F9 };
	};

    class RmluiDebuggerComponentImpl;

	class DARMOK_EXPORT RmluiDebuggerComponent final : public IAppComponent
    {
    public:
		using Config = RmluiDebuggerComponentConfig;
		RmluiDebuggerComponent(const Config& config = {}) noexcept;
		~RmluiDebuggerComponent() noexcept;

		void toggle() noexcept;

		bool isEnabled() const noexcept;

		void init(App& app) override;
		void shutdown() noexcept override;
	private:
		std::unique_ptr<RmluiDebuggerComponentImpl> _impl;
    };
}

#endif

