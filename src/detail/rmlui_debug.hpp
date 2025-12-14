#pragma once

#ifdef _DEBUG

#include <darmok/rmlui_debug.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/window_fwd.hpp>

namespace darmok
{
	class RmluiCanvas;
	class Window;

	class RmluiDebuggerComponentImpl final : ITypeInputEventListener<RmluiDebuggerComponentImpl>
	{
	public:
		using Definition = RmluiDebuggerComponent::Definition;
		RmluiDebuggerComponentImpl(const Definition& def = {}) noexcept;
		~RmluiDebuggerComponentImpl() noexcept;

		expected<void, std::string> init(App& app) noexcept;
		expected<void, std::string> shutdown() noexcept;

		bool isEnabled() const noexcept;
		void toggle() noexcept;

	private:
		OptionalRef<App> _app;
		WindowCursorMode _originalCursorMode;
		Definition _def;
		OptionalRef<RmluiCanvas> _canvas;
		std::vector<std::reference_wrapper<RmluiCanvas>> _canvases;
		static const std::string _tag;
		
		expected<void, std::string> onInputEvent(const std::string& tag) noexcept override;

		void updateCanvases();
	};
}

#endif