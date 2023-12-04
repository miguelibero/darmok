#pragma once

#include <darmok/input.hpp>

#include <dear-imgui/imgui.h>
#include <bgfx/embedded_shader.h>

namespace darmok
{
	class WindowHandle;

	class ImguiContext final
	{
	public:

		static ImguiContext& get();

		bool init();
		bool shutdown();
		void render(bgfx::ViewId viewId, bgfx::TextureHandle texture, ImDrawData* drawData);

		static void* memAlloc(size_t size, void* userData);
		static void memFree(void* ptr, void* userData);

		struct FontRangeMerge
		{
			const void* data;
			size_t      size;
			ImWchar     ranges[3];
		};

		static const std::vector<FontRangeMerge> fontRangeMerge;

		typedef std::array<ImGuiKey, to_underlying(KeyboardKey::Count)> KeyboardMap;
		typedef std::array<ImGuiKey, to_underlying(GamepadButton::Count)> GamepadMap;

		static const KeyboardMap keyboardMap;
		static const GamepadMap gamepadMap;

	private:
		ImguiContext();

		int _initCount;
		bgfx::ProgramHandle _program;
		static const bgfx::EmbeddedShader _embeddedShaders[];
		bgfx::UniformHandle _imageLodEnabled;
		bgfx::ProgramHandle _imageProgram;
		bgfx::VertexLayout  _layout;
		bgfx::UniformHandle _uniform;

		static const uint8_t AlphaBlendFlags;



		static KeyboardMap&& createKeyboardMap();
		static GamepadMap&& createGamepadMap();




	};

    class ImguiViewComponentImpl final
    {
    public:
		ImguiViewComponentImpl(float fontSize = 18.0f);
		void init(bgfx::ViewId viewId);
		void shutdown();

		void beginFrame(const WindowHandle& window, const InputState& input);
		void endFrame();

    private:
		::ImGuiContext* _imgui;
		bgfx::TextureHandle _texture;
		std::array<ImFont*, ImGui::Font::Count> _font;
		int64_t _last;
		int32_t _lastScroll;
		float _fontSize;
		bgfx::ViewId _viewId;


    };

}