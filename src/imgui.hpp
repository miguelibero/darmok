#pragma once

#include <darmok/input.hpp>
#include <darmok/optional_ref.hpp>
#include <imgui.h>
#include <array>
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
		void shutdown();
		void updateLogic(float dt);
		void render(bgfx::ViewId viewId) const;

    private:
		IImguiRenderer& _renderer;
		OptionalRef<App> _app;
		ImGuiContext* _imgui;
		bgfx::TextureHandle _texture;
		std::shared_ptr<Program> _program;
		std::shared_ptr<Program> _imageProgram;
		bgfx::UniformHandle _imageLodEnabledUniform;
		bgfx::UniformHandle _textureUniform;
		static bx::AllocatorI* _alloc;

		void render(bgfx::ViewId viewId, const bgfx::TextureHandle& texture, ImDrawData* drawData) const;
		void updateInput(float dt);
		void beginFrame() const;
		void endFrame(bgfx::ViewId viewId) const;
    };

}