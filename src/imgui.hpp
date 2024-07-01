#pragma once

#include <darmok/input_fwd.hpp>
#include <darmok/optional_ref.hpp>
#include "embedded_shader.hpp"
#include <imgui.h>
#include <unordered_map>
#include <memory>

namespace darmok
{
	class App;
	class IImguiRenderer;
	class Program;

    class ImguiAppComponentImpl final
    {
    public:
		ImguiAppComponentImpl(IImguiRenderer& renderer, float fontSize = 18.0f);

		void init(App& app);
		void shutdown() noexcept;
		void updateLogic(float dt) noexcept;
		bool render(bgfx::ViewId viewId) const noexcept;
		ImGuiContext* getContext() noexcept;

		bool getInputEnabled() const noexcept;
		void setInputEnabled(bool enabled) noexcept;

    private:
		IImguiRenderer& _renderer;
		OptionalRef<App> _app;
		ImGuiContext* _imgui;
		bgfx::TextureHandle _texture;
		std::unique_ptr<Program> _basicProgram;
		std::unique_ptr<Program> _lodProgram;
		bgfx::UniformHandle _lodEnabledUniform;
		bgfx::UniformHandle _textureUniform;
		bool _inputEnabled;

		static const bgfx::EmbeddedShader _embeddedShaders[];
		static const uint8_t _imguiAlphaBlendFlags;
		using KeyboardMap = std::unordered_map<KeyboardKey, ImGuiKey>;
		using GamepadMap = std::unordered_map<GamepadButton, ImGuiKey>;

		static const KeyboardMap& getKeyboardMap() noexcept;
		static const GamepadMap& getGamepadMap() noexcept;

		static void* memAlloc(size_t size, void* userData) noexcept;
		static void memFree(void* ptr, void* userData) noexcept;

		static inline bool checkAvailTransientBuffers(uint32_t numVertices, const bgfx::VertexLayout& layout, uint32_t numIndices) noexcept;

		bool render(bgfx::ViewId viewId, const bgfx::TextureHandle& texture, ImDrawData* drawData) const noexcept;
		void updateInput(float dt) noexcept;
		void beginFrame() const noexcept;
		bool endFrame(bgfx::ViewId viewId) const noexcept;
    };

}