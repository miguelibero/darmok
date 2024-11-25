#pragma once

#include <darmok/app.hpp>
#include <darmok/imgui.hpp>
#include <darmok/optional_ref.hpp>
#include <memory>

namespace darmok
{
    class Scene;
    class ImguiAppComponent;
}

namespace darmok::editor
{
    class EditorAppDelegate final : public darmok::IAppDelegate, public darmok::IImguiRenderer
    {
    public:
		EditorAppDelegate(App& app);
		void init() override;
		void shutdown() override;
        void imguiSetup() override;
		void imguiRender() override;
		void update(float deltaTime) override;
    private:
        App& _app;
        std::shared_ptr<Scene> _scene;
        OptionalRef<ImguiAppComponent> _imgui;
    };
}