#pragma once

#include <glm/glm.hpp>
#include <bgfx/bgfx.h>

namespace darmok
{
    struct Viewport final
    {
        static const glm::uvec4 standardValues;

        glm::uvec2 origin;
        glm::uvec2 size;

        Viewport(const glm::uvec2& size, const glm::uvec2& origin = {}) noexcept;
        Viewport(const glm::uvec4& values = standardValues) noexcept;
        Viewport(glm::uint x, glm::uint y, glm::uint w, glm::uint h) noexcept;

        DLLEXPORT glm::uvec4 getValues() const noexcept;
        DLLEXPORT void setValues(const glm::uvec4& values) noexcept;

        DLLEXPORT glm::vec2 viewportToScreenPoint(const glm::vec2& point) const noexcept;
        DLLEXPORT glm::vec2 screenToViewportPoint(const glm::vec2& point) const noexcept;

        DLLEXPORT glm::vec2 project(const glm::vec2& point) const noexcept;
        DLLEXPORT glm::vec2 unproject(const glm::vec2& point) const noexcept;

        void bgfxSetup(bgfx::ViewId viewId) const noexcept;
    };
}