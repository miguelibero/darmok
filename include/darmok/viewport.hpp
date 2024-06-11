#pragma once

#include <glm/glm.hpp>
#include <bgfx/bgfx.h>
#include <darmok/export.h>

namespace darmok
{
    struct DARMOK_EXPORT Viewport final
    {
        static [[nodiscard]] const glm::uvec4& getStandardValues();

        glm::uvec2 origin;
        glm::uvec2 size;

        Viewport(const glm::uvec2& size, const glm::uvec2& origin = {}) noexcept;
        Viewport(const glm::uvec4& values = getStandardValues()) noexcept;
        Viewport(glm::uint x, glm::uint y, glm::uint w, glm::uint h) noexcept;

        [[nodiscard]] glm::uvec4 getValues() const noexcept;
        void setValues(const glm::uvec4& values) noexcept;

        [[nodiscard]] glm::vec2 viewportToScreenPoint(const glm::vec2& point) const noexcept;
        [[nodiscard]] glm::vec2 screenToViewportPoint(const glm::vec2& point) const noexcept;

        [[nodiscard]] glm::vec2 project(const glm::vec2& point) const noexcept;
        [[nodiscard]] glm::vec2 unproject(const glm::vec2& point) const noexcept;

        void bgfxSetup(bgfx::ViewId viewId) const noexcept;
    };
}