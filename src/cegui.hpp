#pragma once

#include <memory>
#include <bgfx/bgfx.h>

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
    private:
        std::unique_ptr<CeguiRenderer> _renderer;
        std::unique_ptr<CeguiResourceProvider> _resourceProvider;
    };
}