#pragma once

#include <darmok/input.hpp>
#include <darmok/optional_ref.hpp>
#include <dear-imgui/imgui.h>
#include <array>

namespace darmok
{
	class App;
	class IImguiRenderer;

    class ImguiAppComponentImpl final
    {
    public:
		ImguiAppComponentImpl(IImguiRenderer& renderer, float fontSize = 18.0f);

		void init(App& app);
		void shutdown();
		void updateLogic(float dt);
		void render(bgfx::ViewId viewId);

    private:
		IImguiRenderer& _renderer;
		OptionalRef<App> _app;
		ImGuiContext* _imgui;
		bgfx::TextureHandle _texture;
		std::array<ImFont*, ImGui::Font::Count> _font;
		float _fontSize;
		bgfx::ProgramHandle _program;
		bgfx::UniformHandle _imageLodEnabled;
		bgfx::ProgramHandle _imageProgram;
		bgfx::VertexLayout  _layout;
		bgfx::UniformHandle _uniform;
		static bx::AllocatorI* _alloc;

		void render(bgfx::ViewId viewId, const bgfx::TextureHandle& texture, ImDrawData* drawData);
		void updateInput(float dt);
		void beginFrame();
		void endFrame(bgfx::ViewId viewId);
    };

}