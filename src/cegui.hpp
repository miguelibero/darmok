#pragma once

#include <memory>
#include <optional>
#include <string_view>
#include <bgfx/bgfx.h>
#include <darmok/cegui.hpp>
#include <darmok/input.hpp>

namespace CEGUI
{
    class RenderTarget;
}

namespace darmok
{
    class App;
    class CeguiRenderer;
    class CeguiResourceProvider;

    class CeguiAppComponentImpl final : public IKeyboardListener, public IMouseListener
    {
    public:
        CeguiAppComponentImpl() noexcept;
        void init(App& app);
		void shutdown();
		void updateLogic(float deltaTime);
		bgfx::ViewId render(bgfx::ViewId viewId) const;

        void setViewRect(const glm::uvec2& size, const glm::uvec2& origin = glm::uvec2(0)) noexcept;
        void setProjectionFovy(float fovy) noexcept;
        void setFontScale(float scale) noexcept;

        void setResourceGroupDirectory(std::string_view resourceGroup, std::string_view directory) noexcept;
        OptionalRef<CEGUI::GUIContext> getGuiContext() noexcept;
        OptionalRef<const CEGUI::GUIContext> getGuiContext() const noexcept;

        void onKeyboardKey(KeyboardKey key, uint8_t modifiers, bool down) override;
        void onKeyboardChar(const Utf8Char& chr) override;
        void onMouseActive(bool active) override;
        void onMousePositionChange(const glm::vec2& delta) override;
        void onMouseScrollChange(const glm::vec2& delta) override;
        void onMouseButton(MouseButton button, bool down) override;

    private:
        OptionalRef<App> _app;
        OptionalRef<CEGUI::GUIContext> _guiContext;
        std::unique_ptr<CeguiRenderer> _renderer;
        std::unique_ptr<CeguiResourceProvider> _resourceProvider;
        std::optional<glm::uvec2> _viewSize;
        glm::uvec2 _viewOrigin;
        float _projFovy;
        float _fontScale;

        void updateRenderer() const noexcept;
        void updateRenderTarget(CEGUI::RenderTarget& target) const noexcept;
        void updateGuiContext(float deltaTime) noexcept;
    };
}