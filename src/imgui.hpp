#pragma once

#include <darmok/input_fwd.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/render_graph.hpp>
#include <imgui.h>
#include <unordered_map>
#include <memory>
#include <bgfx/bgfx.h>

namespace darmok
{
	class App;
	class IImguiRenderer;
	class Program;

	class ImguiRenderPass final : public IRenderPass
	{
	public:
		ImguiRenderPass(IImguiRenderer& renderer, ImGuiContext* imgui) noexcept;
		~ImguiRenderPass() noexcept;
		void renderPassDefine(RenderPassDefinition& def) noexcept override;
		void renderPassConfigure(bgfx::ViewId viewId) noexcept override;
		void renderPassExecute(IRenderGraphContext& context) noexcept override;
	private:
		static const uint8_t _imguiAlphaBlendFlags;
		IImguiRenderer& _renderer;
		ImGuiContext* _imgui;
		bgfx::ViewId _viewId;
		bgfx::TextureHandle _fontsTexture;
		std::unique_ptr<Program> _program;
		bgfx::UniformHandle _lodEnabledUniform;
		bgfx::UniformHandle _textureUniform;

		void beginFrame() const noexcept;
		bool render(bgfx::Encoder& encoder, ImDrawData* drawData) const noexcept;
		bool endFrame(bgfx::Encoder& encoder) const noexcept;
	};

    class ImguiAppComponentImpl final
    {
    public:
		ImguiAppComponentImpl(IImguiRenderer& renderer, float fontSize = 18.0f);
		~ImguiAppComponentImpl() noexcept;

		void init(App& app);
		void shutdown() noexcept;
		void renderReset() noexcept;
		void update(float dt) noexcept;
		ImGuiContext* getContext() noexcept;

		bool getInputEnabled() const noexcept;
		void setInputEnabled(bool enabled) noexcept;
    private:
		IImguiRenderer& _renderer;
		OptionalRef<App> _app;
		ImGuiContext* _imgui;

		bool _inputEnabled;
		std::optional<ImguiRenderPass> _renderPass;

		using KeyboardMap = std::unordered_map<KeyboardKey, ImGuiKey>;
		using GamepadMap = std::unordered_map<GamepadButton, ImGuiKey>;

		static const KeyboardMap& getKeyboardMap() noexcept;
		static const GamepadMap& getGamepadMap() noexcept;

		static void* memAlloc(size_t size, void* userData) noexcept;
		static void memFree(void* ptr, void* userData) noexcept;

		static inline bool checkAvailTransientBuffers(uint32_t numVertices, const bgfx::VertexLayout& layout, uint32_t numIndices) noexcept;

		void updateInput(float dt) noexcept;

    };

}