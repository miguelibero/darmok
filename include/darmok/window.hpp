#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/color.hpp>
#include <darmok/viewport.hpp>
#include <darmok/window_fwd.hpp>
#include <darmok/expected.hpp>

#include <entt/entt.hpp>
#include <bgfx/bgfx.h>
#include <bx/bx.h>

#include <memory>
#include <string>
#include <filesystem>


namespace darmok
{
	class Platform;

	struct DARMOK_EXPORT VideoMode final
	{
		WindowScreenMode screenMode = WindowScreenMode::Normal;
		glm::uvec2 size = glm::uvec2(0);
		Color3 depth = Color3(0);
		uint16_t refreshRate = 0;
		int monitor = -1;

		std::string toShortString() const noexcept;
		std::string toString() const noexcept;
		bool complete() const noexcept;

		bool operator==(const VideoMode& other) const noexcept;
		bool operator!=(const VideoMode& other) const noexcept;
	};

	struct DARMOK_EXPORT MonitorInfo final
	{
		std::string name;
		Viewport workarea;
	};

	struct DARMOK_EXPORT VideoModeInfo final
	{
		std::vector<VideoMode> modes;
		std::vector<MonitorInfo> monitors;
	};


	class DARMOK_EXPORT BX_NO_VTABLE IWindowListener
	{
	public:
		virtual ~IWindowListener() = default;
		virtual entt::id_type getWindowListenerType() const noexcept { return 0; };
		virtual expected<void, std::string> onWindowSize(const glm::uvec2& size) noexcept { return {}; };
		virtual expected<void, std::string> onWindowPixelSize(const glm::uvec2& size) noexcept { return {}; };
		virtual expected<void, std::string> onWindowPhase(WindowPhase phase) noexcept { return {}; };
		virtual expected<void, std::string> onWindowVideoMode(const VideoMode& mode) noexcept { return {}; };
		virtual expected<void, std::string> onWindowCursorMode(WindowCursorMode mode) noexcept { return {}; };
		virtual expected<void, std::string> onWindowVideoModeInfo(const VideoModeInfo& info) noexcept { return {}; };
		virtual expected<void, std::string> onWindowError(const std::string& error) noexcept { return {}; };
	};

	template<typename T>
	class DARMOK_EXPORT BX_NO_VTABLE ITypeWindowListener : public IWindowListener
	{
	public:
		entt::id_type getWindowListenerType() const noexcept override
		{
			return entt::type_hash<T>::value();
		}
	};

	class DARMOK_EXPORT BX_NO_VTABLE IWindowListenerFilter
	{
	public:
		virtual ~IWindowListenerFilter() = default;
		virtual bool operator()(const IWindowListener& listener) const = 0;
	};

	class WindowImpl;

	enum class FileDialogType
	{
		Open,
		Save
	};

	struct DARMOK_EXPORT FileDialogOptions final
	{
		FileDialogType type = FileDialogType::Open;
		std::string title;
		std::vector<std::string> filters;
		std::filesystem::path defaultPath;
		std::string filterDesc;
		bool allowMultiple = false;
	};

	using FileDialogResult = std::vector<std::filesystem::path>;
	using FileDialogCallback = std::function<expected<void, std::string>(const FileDialogResult&)>;

	class DARMOK_EXPORT Window final
	{
	public:
		Window(Platform& plat) noexcept;
		~Window() noexcept;
		Window(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(const Window&) = delete;
		Window& operator=(Window&&) = delete;

		void requestVideoMode(VideoMode mode) noexcept;
		void requestCursorMode(WindowCursorMode mode) noexcept;
		void requestDestruction() noexcept;
		void requestVideoModeInfo() noexcept;
		void requestTitle(std::string title) noexcept;
		void openFileDialog(FileDialogOptions options, FileDialogCallback callback) noexcept;

		[[nodiscard]] const std::string& getTitle() const noexcept;
		[[nodiscard]] float getAspect() const noexcept;
		[[nodiscard]] const glm::uvec2& getSize() const noexcept;
		[[nodiscard]] const glm::uvec2& getPixelSize() const noexcept;
		[[nodiscard]] WindowPhase getPhase() const noexcept;
		[[nodiscard]] WindowCursorMode getCursorMode() const noexcept;
		[[nodiscard]] const VideoMode& getVideoMode() const noexcept;
		[[nodiscard]] const VideoModeInfo& getVideoModeInfo() const noexcept;

		[[nodiscard]] glm::vec2 windowToScreenPoint(const glm::vec2& point) const noexcept;
		[[nodiscard]] glm::vec2 screenToWindowPoint(const glm::vec2& point) const noexcept;
		[[nodiscard]] glm::vec2 windowToScreenDelta(const glm::vec2& delta) const noexcept;
		[[nodiscard]] glm::vec2 screenToWindowDelta(const glm::vec2& delta) const noexcept;
		[[nodiscard]] glm::vec2 getFramebufferScale() const noexcept;

		[[nodiscard]] const WindowImpl& getImpl() const noexcept;
		[[nodiscard]] WindowImpl& getImpl() noexcept;

		void addListener(std::unique_ptr<IWindowListener> listener) noexcept;
		void addListener(IWindowListener& listener) noexcept;
		bool removeListener(const IWindowListener& listener) noexcept;
		size_t removeListeners(const IWindowListenerFilter& filter) noexcept;

	private:
		std::unique_ptr<WindowImpl> _impl;
	};

} // namespace darmok