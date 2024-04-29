#pragma once

#include <darmok/app.hpp>
#include <darmok/optional_ref.hpp>

namespace CEGUI
{
    class GUIContext;
}

namespace darmok
{
    class CeguiAppComponentImpl;

    class CeguiAppComponent final : public AppComponent
    {
    public:
        CeguiAppComponent() noexcept;
        ~CeguiAppComponent() noexcept;
        void init(App& app) override;
		void shutdown() override;
		void updateLogic(float deltaTime) override;
		bgfx::ViewId render(bgfx::ViewId viewId) const override;

        void setResourceGroupDirectory(std::string_view resourceGroup, std::string_view directory) noexcept;
        OptionalRef<CEGUI::GUIContext> getGuiContext() noexcept;
        OptionalRef<const CEGUI::GUIContext> getGuiContext() const noexcept;

    private:
        std::unique_ptr<CeguiAppComponentImpl> _impl;
    };
}