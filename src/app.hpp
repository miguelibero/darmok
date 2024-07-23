#pragma once

#include "platform.hpp"
#include <darmok/app.hpp>
#include <darmok/input.hpp>
#include <darmok/window.hpp>
#include <darmok/asset.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/render_graph.hpp>
#include <string>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>

namespace darmok
{
	class AppImpl final : public IKeyboardListener
	{
	public:
		AppImpl(App& app) noexcept;
		void setConfig(const AppConfig& config) noexcept;
		void init();
		void updateLogic(float deltaTime);
		void render() const;
		AppUpdateResult processEvents();
		void shutdown();

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
		
		AppUpdateResult _updateResult;
		bool _running;
		uint32_t _debug;
		uint64_t _lastUpdate;
		AppConfig _config;
		Platform& _plat;
		App& _app;
		Input _input;
		Window _window;
		AssetContext _assets;
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