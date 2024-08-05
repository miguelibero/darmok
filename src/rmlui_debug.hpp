#pragma once

#ifdef _DEBUG

#include <darmok/rmlui_debug.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok
{
	class RmluiView;

	class RmluiDebuggerAppComponentImpl final : IInputEventListener
	{
	public:
		using Config = RmluiDebuggerAppComponentConfig;
		RmluiDebuggerAppComponentImpl(RmluiAppComponent& comp, const Config& config = {}) noexcept;
		~RmluiDebuggerAppComponentImpl() noexcept;

		void init(App& app) noexcept;
		void shutdown() noexcept;

		bool isEnabled() const noexcept;
		void toggle() noexcept;

	private:
		OptionalRef<Input> _input;
		RmluiAppComponent& _comp;
		Config _config;
		OptionalRef<RmluiView> _view;
		static const std::string _tag;
		
		void onInputEvent(const std::string& tag) noexcept override;
	};
}

#endif