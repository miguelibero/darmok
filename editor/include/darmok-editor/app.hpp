#pragma once

#include <darmok/app.hpp>
#include <darmok/imgui.hpp>

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
    };
}