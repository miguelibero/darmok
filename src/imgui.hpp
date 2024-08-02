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

    class ImguiAppComponentImpl final : public IRenderPass
    {
    public:
		ImguiAppComponentImpl(IImguiRenderer& renderer, float fontSize = 18.0f);

		void init(App& app);
		void shutdown() noexcept;
		void renderReset() noexcept;
		void updateLogic(float dt) noexcept;
		ImGuiContext* getContext() noexcept;

		bool getInputEnabled() const noexcept;
		void setInputEnabled(bool enabled) noexcept;

		void renderPassDefine(RenderPassDefinition& def) noexcept override;
		void renderPassConfigure(bgfx::ViewId viewId) noexcept override;
		void renderPassExecute(IRenderGraphContext& context) noexcept override;

    private:
		IImguiRenderer& _renderer;
		OptionalRef<App> _app;
		ImGuiContext* _imgui;
		bgfx::TextureHandle _fonts;
		std::unique_ptr<Program> _program;
		bgfx::UniformHandle _lodEnabledUniform;
		bgfx::UniformHandle _textureUniform;
		bool _inputEnabled;
		bgfx::ViewId _viewId;

		static const uint8_t _imguiAlphaBlendFlags;
		using KeyboardMap = std::unordered_map<KeyboardKey, ImGuiKey>;
		using GamepadMap = std::unordered_map<GamepadButton, ImGuiKey>;

		static const KeyboardMap& getKeyboardMap() noexcept;
		static const GamepadMap& getGamepadMap() noexcept;

		static void* memAlloc(size_t size, void* userData) noexcept;
		static void memFree(void* ptr, void* userData) noexcept;

		static inline bool checkAvailTransientBuffers(uint32_t numVertices, const bgfx::VertexLayout& layout, uint32_t numIndices) noexcept;

		bool render(bgfx::Encoder& encoder, ImDrawData* drawData) const noexcept;
		void updateInput(float dt) noexcept;
		void beginFrame() const noexcept;
		bool endFrame(bgfx::Encoder& encoder) const noexcept;
    };

}