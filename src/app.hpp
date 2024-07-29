#pragma once

#include "platform.hpp"
#include <darmok/app.hpp>
#include <darmok/input.hpp>
#include <darmok/window.hpp>
#include <darmok/asset.hpp>
#include <darmok/audio.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/render_graph.hpp>
#include <darmok/data.hpp>
#include <string>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>

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

	class AppImpl final : public IKeyboardListener, public IWindowListener
	{
	public:
		AppImpl(App& app) noexcept;
		void setConfig(const AppConfig& config) noexcept;
		void init();
		void updateLogic(float deltaTime);
		void render() const;
		AppUpdateResult processEvents();
		void shutdown();

		void onWindowPixelSize(const glm::uvec2& size) override;
		void onKeyboardKey(KeyboardKey key, const KeyboardModifiers& modifiers, bool down) override;

		bool toggleDebugFlag(uint32_t flag) noexcept;
		void setDebugFlag(uint32_t flag, bool enabled = true) noexcept;

		void addComponent(std::unique_ptr<AppComponent>&& component) noexcept;
		
		[[nodiscard]] Input& getInput() noexcept;
		[[nodiscard]] const Input& getInput() const noexcept;
		[[nodiscard]] Window& getWindow() noexcept;
		[[nodiscard]] const Window& getWindow() const noexcept;
		[[nodiscard]] AssetContext& getAssets() noexcept;
		[[nodiscard]] const AssetContext& getAssets() const noexcept;
		[[nodiscard]] Platform& getPlatform() noexcept;
		[[nodiscard]] const Platform& getPlatform() const noexcept;
		[[nodiscard]] RenderGraphDefinition& getRenderGraph() noexcept;
		[[nodiscard]] const RenderGraphDefinition& getRenderGraph() const noexcept;

#ifdef DARMOK_MINIAUDIO
		[[nodiscard]] AudioSystem& getAudio() noexcept;
		[[nodiscard]] const AudioSystem& getAudio() const noexcept;
#endif

		template <typename F>
		void update(const F& logicCallback)
		{
			if (getWindow().getPhase() != WindowPhase::Running)
			{
				return;
			}
			auto timePassed = updateTimePassed();
			const auto dt = _config.targetUpdateDeltaTime;
			auto i = 0;
			while (timePassed > dt && i < _config.maxInstantLogicUpdates)
			{
				logicCallback(dt);
				timePassed -= dt;
				i++;
			}
			logicCallback(timePassed);
		}

	private:
		[[nodiscard]] float updateTimePassed() noexcept;
		[[nodiscard]] bool getDebugFlag(uint32_t flag) const noexcept;

		void handleDebugShortcuts(KeyboardKey key, const KeyboardModifiers& modifiers);

		// first since it contains the allocator
		AssetContext _assets;
		
		AppUpdateResult _updateResult;
		bool _running;
		uint32_t _debug;
		uint64_t _lastUpdate;
		AppConfig _config;
		Platform& _plat;
		App& _app;
		Input _input;
		Window _window;
#ifdef DARMOK_MINIAUDIO
		AudioSystem _audio;
#endif
		RenderGraphDefinition _renderGraphDef;
		std::optional<RenderGraph> _renderGraph;

		std::vector<std::unique_ptr<AppComponent>> _components;
	};

	class AppRunner final : public IPlatformRunnable
	{
	public:
		AppRunner(std::unique_ptr<App>&& app) noexcept;
		int32_t operator()() noexcept override;
	private:
		std::unique_ptr<App> _app;
		bool init() noexcept;
		AppUpdateResult update() noexcept;
		bool shutdown() noexcept;
	};
}