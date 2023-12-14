#pragma once

#include <darmok/input.hpp>

#include <dear-imgui/imgui.h>
#include <bx/bx.h>

namespace darmok
{
	struct WindowHandle;
	class IImguiRenderer;

	class ImguiContext final
	{
	public:

		static ImguiContext& get();

		bool init();
		bool shutdown();
		void render(bgfx::ViewId viewId, const bgfx::TextureHandle& texture, ImDrawData* drawData);

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
		bgfx::UniformHandle _imageLodEnabled;
		bgfx::ProgramHandle _imageProgram;
		bgfx::VertexLayout  _layout;
		bgfx::UniformHandle _uniform;

		static const uint8_t AlphaBlendFlags;

		static KeyboardMap createKeyboardMap();
		static GamepadMap createGamepadMap();
	};

    class ImguiViewComponentImpl final
    {
    public:
		ImguiViewComponentImpl(IImguiRenderer& renderer, float fontSize = 18.0f);

		void init(bgfx::ViewId viewId);
		void shutdown();
		void updateLogic(float dt);
		void render();

    private:
		IImguiRenderer& _renderer;
		::ImGuiContext* _imgui;
		bgfx::TextureHandle _texture;
		std::array<ImFont*, ImGui::Font::Count> _font;
		int32_t _lastScroll;
		float _fontSize;
		bgfx::ViewId _viewId;

		void updateInput(float dt);
		void beginFrame();
		void endFrame();
    };

}