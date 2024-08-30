#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <bgfx/bgfx.h>
#include <bx/bx.h>

namespace darmok
{
    struct DARMOK_EXPORT Viewport final
    {
        glm::ivec2 origin;
        glm::ivec2 size;

        Viewport() noexcept;
        Viewport(const glm::ivec2& size, const glm::ivec2& origin = {}) noexcept;
        Viewport(const glm::uvec2& size) noexcept;
        Viewport(const glm::ivec4& values) noexcept;
        Viewport(int x, int y, int w, int h) noexcept;

        bool operator==(const Viewport& other) const noexcept;
        bool operator!=(const Viewport& other) const noexcept;

        float getAspectRatio() const noexcept;

        [[nodiscard]] glm::ivec4 getValues() const noexcept;
        Viewport& setValues(const glm::ivec4& values) noexcept;

        [[nodiscard]] glm::vec2 viewportToScreenPoint(const glm::vec2& point) const noexcept;
        [[nodiscard]] glm::vec2 screenToViewportPoint(const glm::vec2& point) const noexcept;

        [[nodiscard]] glm::vec2 viewportToScreenDelta(const glm::vec2& delta) const noexcept;
        [[nodiscard]] glm::vec2 screenToViewportDelta(const glm::vec2& delta) const noexcept;

        [[nodiscard]] glm::vec2 project(const glm::vec2& point) const noexcept;
        [[nodiscard]] glm::vec2 unproject(const glm::vec2& point) const noexcept;

        void configureView(bgfx::ViewId viewId) const noexcept;
        glm::mat4 ortho(const glm::vec2& center = glm::vec2(0.5f), float near = -bx::kFloatLargest, float far = bx::kFloatLargest) const noexcept;
    };
}