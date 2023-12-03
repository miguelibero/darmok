#pragma once

#include <darmok/input.hpp>

#include <dear-imgui/imgui.h>
#include <bgfx/embedded_shader.h>

namespace darmok
{
	class WindowHandle;

    class ImguiAppComponentImpl final
    {
    public:
		ImguiAppComponentImpl(float fontSize = 18.0f);
		void init();
		void beginFrame(const WindowHandle& window, const InputState& input);
		void endFrame(bgfx::ViewId viewId);

		void shutdown();

		static void* memAlloc(size_t size, void* userData);
		static void memFree(void* ptr, void* userData);

    private:

		typedef std::array<ImGuiKey, to_underlying(KeyboardKey::Count)> KeyboardMap;
		typedef std::array<ImGuiKey, to_underlying(GamepadButton::Count)> GamepadMap;

		static KeyboardMap&& createKeyboardMap();
		static GamepadMap&& createGamepadMap();

        static void render(bgfx::ViewId viewId, ImDrawData* drawData);
		static void setupStyle(bool dark);

		struct FontRangeMerge
		{
			const void* data;
			size_t      size;
			ImWchar     ranges[3];
		};

		::ImGuiContext* _imgui;
		bgfx::VertexLayout  _layout;
		bgfx::ProgramHandle _program;
		bgfx::ProgramHandle _imageProgram;
		bgfx::TextureHandle _texture;
		bgfx::UniformHandle _uniform;
		bgfx::UniformHandle _imageLodEnabled;
		std::array<ImFont*, ImGui::Font::Count> _font;
		int64_t _last;
		int32_t _lastScroll;
		float _fontSize;

		static const KeyboardMap _keyboardMap;
		static const GamepadMap _gamepadMap;

		static const uint8_t AlphaBlendFlags;
		static const bgfx::EmbeddedShader _embeddedShaders[];
		static const FontRangeMerge _fontRangeMerge[];
    };

}