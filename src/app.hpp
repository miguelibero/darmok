#pragma once

#include "platform.hpp"
#include <darmok/app.hpp>
#include <darmok/input.hpp>
#include <darmok/window.hpp>
#include <darmok/asset.hpp>
#include <darmok/audio.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/render_graph.hpp>
#include <darmok/render_chain.hpp>
#include <darmok/data.hpp>
#include <darmok/color.hpp>
#include <string>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>
#include <taskflow/taskflow.hpp>

namespace darmok
{
	class BgfxFatalException : public std::exception
	{
	public:
		const char* filePath;
		uint16_t line;
		bgfx::Fatal::Enum code;
		BgfxFatalException(const char* filePath, uint16_t line, bgfx::Fatal::Enum code, const char* msg);
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

		uint32_t cacheReadSize(uint64_t id) override;

		bool cacheRead(uint64_t id, void* data, uint32_t size) override;

		void cacheWrite(uint64_t id, const void* data, uint32_t size) override;

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
		BgfxCallbacks() = default;
	};

	class AppImpl final : IKeyboardListener, IRenderPass
	{
	public:
		AppImpl(App& app) noexcept;
		void init();
		void update(float deltaTime);
		void afterUpdate(float deltaTime);
		void render() const;
		AppRunResult processEvents();
		void shutdown();

		bool toggleDebugFlag(uint32_t flag) noexcept;
		void setDebugFlag(uint32_t flag, bool enabled = true) noexcept;
		bool getDebugFlag(uint32_t flag) const noexcept;
		
		bool toggleResetFlag(uint32_t flag) noexcept;
		void setResetFlag(uint32_t flag, bool enabled = true) noexcept;
		bool getResetFlag(uint32_t flag) const noexcept;

		void setClearColor(const Color& color) noexcept;
		void setUpdateConfig(const AppUpdateConfig& config) noexcept;

		void setPaused(bool paused) noexcept;
		bool isPaused() const noexcept;

		void addComponent(entt::id_type type, std::unique_ptr<IAppComponent>&& component) noexcept;
		bool removeComponent(entt::id_type type) noexcept;
		bool hasComponent(entt::id_type type) const noexcept;
		OptionalRef<IAppComponent> getComponent(entt::id_type type) noexcept;
		OptionalRef<const IAppComponent> getComponent(entt::id_type type) const noexcept;

		Input& getInput() noexcept;
		const Input& getInput() const noexcept;
		Window& getWindow() noexcept;
		const Window& getWindow() const noexcept;
		AssetContext& getAssets() noexcept;
		const AssetContext& getAssets() const noexcept;
		Platform& getPlatform() noexcept;
		const Platform& getPlatform() const noexcept;
		RenderGraphDefinition& getRenderGraph() noexcept;
		const RenderGraphDefinition& getRenderGraph() const noexcept;
		tf::Executor& getTaskExecutor();
		const tf::Executor& getTaskExecutor() const;

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
			const auto dt = _updateConfig.deltaTime;
			auto i = 0;
			while (timePassed > dt && i < _updateConfig.maxInstant)
			{
				logicCallback(dt);
				timePassed -= dt;
				i++;
			}
			logicCallback(timePassed);
		}

	private:
		using Components = std::vector<std::pair<entt::id_type, std::unique_ptr<IAppComponent>>>;

		Components::iterator findComponent(entt::id_type type) noexcept;
		Components::const_iterator findComponent(entt::id_type type) const noexcept;

		[[nodiscard]] float updateTimePassed() noexcept;

		void handleDebugShortcuts(KeyboardKey key, const KeyboardModifiers& modifiers);
		void toggleTaskflowProfile();

		void renderPassConfigure(bgfx::ViewId viewId) override;
		void renderPassExecute(IRenderGraphContext& context) override;
		void renderPassDefine(RenderPassDefinition& def) override;

		void renderReset();
		void bgfxInit();
		void onKeyboardKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down) override;

		// first since it contains the allocator
		AssetContext _assets;

		AppRunResult _runResult;
		bool _running;
		bool _paused;
		glm::uvec2 _pixelSize;
		VideoMode _videoMode;
		uint32_t _debugFlags;
		uint32_t _resetFlags;
		uint32_t _activeResetFlags;
		Color _clearColor;
		uint64_t _lastUpdate;
		AppUpdateConfig _updateConfig;
		Platform& _plat;
		App& _app;
		Input _input;
		Window _window;
#ifdef DARMOK_MINIAUDIO
		AudioSystem _audio;
#endif
		mutable std::optional<tf::Executor> _taskExecutor;
		std::shared_ptr<tf::TFProfObserver> _taskObserver;

		RenderGraphDefinition _renderGraphDef;
		std::optional<RenderGraph> _renderGraph;

		Components _components;
	};

	class AppRunner final : public IPlatformRunnable
	{
	public:
		AppRunner(std::unique_ptr<App>&& app) noexcept;
		int32_t operator()() noexcept override;
	private:
		std::unique_ptr<App> _app;
		bool init() noexcept;
		AppRunResult run() noexcept;
		bool shutdown() noexcept;
	};
}