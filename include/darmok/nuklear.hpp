#pragma once

#include <darmok/app.hpp>
#include <darmok/optional_ref.hpp>
#include <memory>
#include <string_view>
#include <bx/bx.h>

#include <nuklear.h>

namespace darmok
{
    class BX_NO_VTABLE INuklearRenderer
    {
    public:
        virtual ~INuklearRenderer() = default;
        virtual void nuklearRender(nk_context& ctx) = 0;
    };


    class NuklearAppComponentImpl;

    class NuklearAppComponent final : public AppComponent
    {
    public:
        NuklearAppComponent(INuklearRenderer& renderer) noexcept;
        ~NuklearAppComponent() noexcept;
        void init(App& app) override;
		void shutdown() override;
		void updateLogic(float deltaTime) override;
		bgfx::ViewId render(bgfx::ViewId viewId) override;

        nk_context& getContext() noexcept;
        const nk_context& getContext() const noexcept;

        OptionalRef<nk_user_font> loadFont(std::string_view name, float height = 18) noexcept;
    private:
        std::unique_ptr<NuklearAppComponentImpl> _impl;
    };
}