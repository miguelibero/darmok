#pragma once

#ifdef _DEBUG

#include <darmok/rmlui_debug.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/window_fwd.hpp>

namespace darmok
{
	class RmluiCanvas;
	class Window;

	class RmluiDebuggerComponentImpl final : IInputEventListener
	{
	public:
		using Config = RmluiDebuggerComponentConfig;
		RmluiDebuggerComponentImpl(const Config& config = {}) noexcept;
		~RmluiDebuggerComponentImpl() noexcept;

		void init(Scene& scene, App& app) noexcept;
		void shutdown() noexcept;

		bool isEnabled() const noexcept;
		void toggle() noexcept;

	private:
		OptionalRef<Scene> _scene;
		OptionalRef<Input> _input;
		OptionalRef<Window> _win;
		WindowCursorMode _originalCursorMode;
		Config _config;
		OptionalRef<RmluiCanvas> _canvas;
		static const std::string _tag;
		
		void onInputEvent(const std::string& tag) noexcept override;
	};
}

#endif