#pragma once

#include "platform.hpp"

#include <darmok/app.hpp>
#include <darmok/input.hpp>
#include <darmok/window.hpp>
#include <darmok/asset.hpp>
#include <darmok/audio.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/render_chain.hpp>
#include <darmok/data.hpp>
#include <darmok/color.hpp>
#include <darmok/utils.hpp>

#include <taskflow/taskflow.hpp>

#include <string>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>

namespace darmok
{
	class BgfxFatalException : public std::runtime_error
	{
	public:
		BgfxFatalException(const char* filePath, uint16_t line, bgfx::Fatal::Enum code, const char* msg);
		[[nodiscard]] const char* getFilePath() const noexcept;
		[[nodiscard]] uint16_t getLine() const noexcept;
		[[nodiscard]] bgfx::Fatal::Enum getCode() const noexcept;
	private:
		const char* _filePath;
		uint16_t _line;
		bgfx::Fatal::Enum _code;

	};

	class BgfxCallbacks final : public bgfx::CallbackI
	{
	public:

		static BgfxCallbacks& get() noexcept;

		void fatal(
			const char* filePath
			, uint16_t line
			, bgfx::Fatal::Enum code
			, const char* str
		) override;

		void traceVargs(
			const char* filePath
			, uint16_t line
			, const char* format
			, va_list argList
		) override;

		void profilerBegin(
			const char* name
			, uint32_t abgr
			, const char* filePath
			, uint16_t line
		) override;

		void profilerBeginLiteral(
			const char* name
			, uint32_t abgr
			, const char* filePath
			, uint16_t line
		) override;

		void profilerEnd() override;

		uint32_t cacheReadSize(uint64_t resId) override;

		bool cacheRead(uint64_t resId, void* data, uint32_t size) override;

		void cacheWrite(uint64_t resId, const void* data, uint32_t size) override;

		void screenShot(
			const char* filePath
			, uint32_t width
			, uint32_t height
			, uint32_t pitch
			, const void* data
			, uint32_t size
			, bool yflip
		) override;

		void captureBegin(
			uint32_t width
			, uint32_t height
			, uint32_t pitch
			, bgfx::TextureFormat::Enum format
			, bool yflip
		) override;

		void captureEnd() override;

		void captureFrame(const void* data, uint32_t size) override;
	private:
		std::mutex _cacheMutex;
		std::unordered_map<uint64_t, Data> _cache;
		BgfxCallbacks() noexcept;
	};

	class AppImpl final : ITypeKeyboardListener<AppImpl>
	{
	public:
		AppImpl(App& app, std::unique_ptr<IAppDelegateFactory>&& factory) noexcept;
		~AppImpl() = default;
		AppImpl(const AppImpl&) = delete;
		AppImpl(AppImpl&&) = delete;
		AppImpl& operator=(const AppImpl&) = delete;
		AppImpl& operator=(AppImpl&&) = delete;

		std::optional<int32_t> setup(const CmdArgs& args);
		void init();
		void update(float deltaTime);
		void render() const;
		void requestRenderReset();
		AppRunResult processEvents();
		void shutdown();
		void quit() noexcept;

		void onException(AppPhase phase, const std::exception& exc) noexcept;

		bool toggleDebugFlag(uint32_t flag) noexcept;
		void setDebugFlag(uint32_t flag, bool enabled = true) noexcept;
		[[nodiscard]] bool getDebugFlag(uint32_t flag) const noexcept;
		
		bool toggleResetFlag(uint32_t flag) noexcept;
		void setResetFlag(uint32_t flag, bool enabled = true) noexcept;
		[[nodiscard]] bool getResetFlag(uint32_t flag) const noexcept;

		void setClearColor(const Color& color) noexcept;
		void setUpdateConfig(const AppUpdateConfig& config) noexcept;

		void setRendererType(bgfx::RendererType::Enum renderer);

		void setPaused(bool paused) noexcept;
		[[nodiscard]] bool isPaused() const noexcept;

		void addUpdater(std::unique_ptr<IAppUpdater>&& updater) noexcept;
		void addUpdater(IAppUpdater& updater) noexcept;
		bool removeUpdater(const IAppUpdater& updater) noexcept;
		size_t removeUpdaters(const IAppUpdaterFilter& filter) noexcept;

		void addComponent(std::unique_ptr<IAppComponent>&& component) noexcept;
		bool removeComponent(entt::id_type type) noexcept;
		[[nodiscard]] bool hasComponent(entt::id_type type) const noexcept;
		[[nodiscard]] OptionalRef<IAppComponent> getComponent(entt::id_type type) noexcept;
		[[nodiscard]] OptionalRef<const IAppComponent> getComponent(entt::id_type type) const noexcept;

		[[nodiscard]] Input& getInput() noexcept;
		[[nodiscard]] const Input& getInput() const noexcept;
		[[nodiscard]] Window& getWindow() noexcept;
		[[nodiscard]] const Window& getWindow() const noexcept;
		[[nodiscard]] AssetContext& getAssets() noexcept;
		[[nodiscard]] const AssetContext& getAssets() const noexcept;
		[[nodiscard]] Platform& getPlatform() noexcept;
		[[nodiscard]] const Platform& getPlatform() const noexcept;
		[[nodiscard]] tf::Executor& getTaskExecutor();
		[[nodiscard]] const tf::Executor& getTaskExecutor() const;

#ifdef DARMOK_MINIAUDIO
		AudioSystem& getAudio() noexcept;
		const AudioSystem& getAudio() const noexcept;
#endif

		template <typename F>
		void deltaTimeCall(const F& logicCallback)
		{
			if (getWindow().getPhase() != WindowPhase::Running)
			{
				return;
			}
			auto timePassed = updateTimePassed();
			const auto deltaTime = _updateConfig.deltaTime;
			auto i = 0;
			while (timePassed > deltaTime && i < _updateConfig.maxInstant)
			{
				logicCallback(deltaTime);
				timePassed -= deltaTime;
				i++;
			}
			logicCallback(timePassed);
		}

	private:
		using Components = std::vector<std::shared_ptr<IAppComponent>>;

		[[nodiscard]] Components::iterator findComponent(entt::id_type type) noexcept;
		[[nodiscard]] Components::const_iterator findComponent(entt::id_type type) const noexcept;

		[[nodiscard]] float updateTimePassed() noexcept;

		void handleDebugShortcuts(KeyboardKey key, const KeyboardModifiers& modifiers);
		void toggleTaskflowProfile();

		void bgfxInit();
		void onKeyboardKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down) override;
		void setNextRenderer();
		void requestNextVideoMode();
		void renderReset();

		static const std::vector<bgfx::RendererType::Enum>& getSupportedRenderers() noexcept;

		// first since it contains the allocator
		AssetContext _assets;

		AppRunResult _runResult;
		bool _running;
		bool _paused;
		bool _renderReset;
		glm::uvec2 _renderSize;
		VideoMode _videoMode;
		uint32_t _debugFlags;
		uint32_t _resetFlags;
		bgfx::RendererType::Enum _rendererType;
		uint32_t _activeResetFlags;
		Color _clearColor;
		uint64_t _lastUpdate;
		AppUpdateConfig _updateConfig;
		Platform& _plat;
		App& _app;
		std::unique_ptr<IAppDelegate> _delegate;
		std::unique_ptr<IAppDelegateFactory> _delegateFactory;
		Input _input;
		Window _window;
#ifdef DARMOK_MINIAUDIO
		AudioSystem _audio;
#endif
		mutable std::optional<tf::Executor> _taskExecutor;
		std::shared_ptr<tf::TFProfObserver> _taskObserver;

		OwnRefCollection<IAppUpdater> _updaters;

		Components _components;
	};

	class AppRunner final : public IPlatformRunnable
	{
	public:
		AppRunner(std::unique_ptr<App>&& app, const CmdArgs& args) noexcept;
		std::optional<int32_t> setup() noexcept;
		int32_t operator()() noexcept override;
	private:
		bool _setupDone;
		std::unique_ptr<App> _app;
		CmdArgs _args;
		bool init() noexcept;
		AppRunResult run() noexcept;
		bool shutdown() noexcept;
	};
}