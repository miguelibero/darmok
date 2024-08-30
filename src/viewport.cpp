#include <darmok/viewport.hpp>
#include <darmok/math.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <bx/math.h>

namespace darmok
{
    Viewport::Viewport() noexcept
        : size(1)
        , origin(0)
    {
    }

    Viewport::Viewport(const glm::uvec2& size) noexcept
        : size(size)
        , origin(0)
    {
    }

    Viewport::Viewport(const glm::ivec2& size, const glm::ivec2& origin) noexcept
        : size(size)
        , origin(origin)
    {
    }

    Viewport::Viewport(const glm::ivec4& values) noexcept
        : size(values[2], values[3])
        , origin(values[0], values[1])
    {
    }

    Viewport::Viewport(int x, int y, int w, int h) noexcept
        : size(w, h)
        , origin(x, y)
    {
    }

    bool Viewport::operator==(const Viewport& other) const noexcept
    {
        return origin == other.origin && size == other.size;
    }

    bool Viewport::operator!=(const Viewport& other) const noexcept
    {
        return !operator==(other);
    }

    float Viewport::getAspectRatio() const noexcept
    {
        return (float)size.x / size.y;
    }

    glm::ivec4 Viewport::getValues() const noexcept
    {
        return glm::ivec4(origin.x, origin.y, size.x, size.y);
    }

    Viewport& Viewport::setValues(const glm::ivec4& values) noexcept
    {
        origin.x = values[0];
        origin.y = values[1];
        size.x = values[2];
        size.y = values[3];

        return *this;
    }

    glm::vec2 Viewport::screenToViewportPoint(const glm::vec2& point) const noexcept
    {
        return (point - glm::vec2(origin)) / glm::vec2(size);
    }

    glm::vec2 Viewport::viewportToScreenPoint(const glm::vec2& point) const noexcept
    {
        return glm::vec2(origin) + (point * glm::vec2(size));
    }

    glm::vec2 Viewport::viewportToScreenDelta(const glm::vec2& delta) const noexcept
    {
        return delta * glm::vec2(size);
    }

    glm::vec2 Viewport::screenToViewportDelta(const glm::vec2& delta) const noexcept
    {
        return delta / glm::vec2(size);
    }

    glm::vec2 Viewport::project(const glm::vec2& point) const noexcept
    {
        auto p = (point * 0.5F) + 0.5F;
        return viewportToScreenPoint(p);
    }

    glm::vec2 Viewport::unproject(const glm::vec2& point) const noexcept
    {
        auto p = screenToViewportPoint(point);
        return (p * 2.F) - 1.F;
    }

    void Viewport::configureView(bgfx::ViewId viewId) const noexcept
    {
        bgfx::setViewRect(viewId, origin.x, origin.y, size.x, size.y);
    }

    glm::mat4 Viewport::ortho(const glm::vec2& center, float near, float far) const noexcept
    {
        glm::vec2 a(origin);
        glm::vec2 s(size);
        auto botLeft = a - (center * s);
        auto topRight = a + ((glm::vec2(1) - center) * s);
        return Math::ortho(botLeft, topRight, near, far);
    }
}