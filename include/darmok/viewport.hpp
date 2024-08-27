#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <bgfx/bgfx.h>
#include <bx/bx.h>

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

        bool operator==(const Viewport& other) const noexcept;
        bool operator!=(const Viewport& other) const noexcept;

        [[nodiscard]] glm::uvec4 getValues() const noexcept;
        void setValues(const glm::uvec4& values) noexcept;

        [[nodiscard]] glm::vec2 viewportToScreenPoint(const glm::vec2& point) const noexcept;
        [[nodiscard]] glm::vec2 screenToViewportPoint(const glm::vec2& point) const noexcept;

        [[nodiscard]] glm::vec2 project(const glm::vec2& point) const noexcept;
        [[nodiscard]] glm::vec2 unproject(const glm::vec2& point) const noexcept;

        void configureView(bgfx::ViewId viewId) const noexcept;
        glm::mat4 ortho(const glm::vec2& center = glm::vec2(0.5f), float near = -bx::kFloatLargest, float far = bx::kFloatLargest) const noexcept;
    };
}