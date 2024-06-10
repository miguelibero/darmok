#pragma once

#include <glm/glm.hpp>
#include <bgfx/bgfx.h>

namespace darmok
{
    struct Viewport final
    {
        DLLEXPORT static [[nodiscard]] const glm::uvec4& getStandardValues();

        glm::uvec2 origin;
        glm::uvec2 size;

        DLLEXPORT Viewport(const glm::uvec2& size, const glm::uvec2& origin = {}) noexcept;
        DLLEXPORT Viewport(const glm::uvec4& values = getStandardValues()) noexcept;
        DLLEXPORT Viewport(glm::uint x, glm::uint y, glm::uint w, glm::uint h) noexcept;

        DLLEXPORT [[nodiscard]] glm::uvec4 getValues() const noexcept;
        DLLEXPORT void setValues(const glm::uvec4& values) noexcept;

        DLLEXPORT [[nodiscard]] glm::vec2 viewportToScreenPoint(const glm::vec2& point) const noexcept;
        DLLEXPORT [[nodiscard]] glm::vec2 screenToViewportPoint(const glm::vec2& point) const noexcept;

        DLLEXPORT [[nodiscard]] glm::vec2 project(const glm::vec2& point) const noexcept;
        DLLEXPORT [[nodiscard]] glm::vec2 unproject(const glm::vec2& point) const noexcept;

        DLLEXPORT void bgfxSetup(bgfx::ViewId viewId) const noexcept;
    };
}