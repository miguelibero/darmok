#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/math.hpp>
#include <bgfx/bgfx.h>
#include <bx/bx.h>

namespace darmok
{
    struct DARMOK_EXPORT Viewport final
    {
        glm::uvec2 origin;
        glm::uvec2 size;

        Viewport() noexcept;
        Viewport(const glm::uvec2& size, const glm::uvec2& origin = {}) noexcept;
        Viewport(const glm::uvec4& values) noexcept;
        Viewport(glm::uint x, glm::uint y, glm::uint w, glm::uint h) noexcept;

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

        void configureView(bgfx::ViewId viewId) const;

        glm::mat4 ortho(const glm::vec2& center = glm::vec2(0.5f), float near = Math::defaultNear, float far = Math::defaultFar) const noexcept;
    };
}