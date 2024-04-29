#pragma once

#include <memory>
#include <string_view>
#include <bgfx/bgfx.h>
#include <darmok/cegui.hpp>

namespace darmok
{
    class App;
    class CeguiRenderer;
    class CeguiResourceProvider;

    class CeguiAppComponentImpl final
    {
    public:
        CeguiAppComponentImpl() noexcept;
        void init(App& app);
		void shutdown();
		void updateLogic(float deltaTime);
		bgfx::ViewId render(bgfx::ViewId viewId) const;

        void setResourceGroupDirectory(std::string_view resourceGroup, std::string_view directory) noexcept;
        OptionalRef<CEGUI::GUIContext> getGuiContext() noexcept;
        OptionalRef<const CEGUI::GUIContext> getGuiContext() const noexcept;

    private:
        OptionalRef<CEGUI::GUIContext> _guiContext;
        std::unique_ptr<CeguiRenderer> _renderer;
        std::unique_ptr<CeguiResourceProvider> _resourceProvider;
    };
}