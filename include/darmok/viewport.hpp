#pragma once

#include <glm/glm.hpp>
#include <bgfx/bgfx.h>

namespace darmok
{
    struct Viewport final
    {
        static const glm::ivec4 standardValues;

        glm::ivec4 values;

        Viewport(const glm::uvec2& size) noexcept;
        Viewport(const glm::ivec4& values = standardValues) noexcept;
        Viewport(int x, int y, int w, int h) noexcept;

        DLLEXPORT glm::uvec2 getSize() const noexcept;
        DLLEXPORT glm::ivec2 getOrigin() const noexcept;

        DLLEXPORT void setSize(const glm::uvec2& size) noexcept;
        DLLEXPORT void setOrigin(const glm::ivec2& origin) noexcept;

        DLLEXPORT glm::vec2 viewportToScreenPoint(const glm::vec2& point) const noexcept;
        DLLEXPORT glm::vec2 screenToViewportPoint(const glm::vec2& point) const noexcept;

        DLLEXPORT glm::mat4 getOrtho(float near = -1000, float far = 1000) const noexcept;

        DLLEXPORT glm::vec2 adaptFromScreenPoint(const glm::vec2& point, const glm::vec2& size) const noexcept;

        void bgfxSetup(bgfx::ViewId viewId) const noexcept;
    };
}