#pragma once

#include <darmok/app.hpp>

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
    private:
        std::unique_ptr<CeguiAppComponentImpl> _impl;
    };
}