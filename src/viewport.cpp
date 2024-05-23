#include <darmok/viewport.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <bx/math.h>

namespace darmok
{
    const glm::uvec4 Viewport::standardValues(0, 0, 1, 1);

    Viewport::Viewport(const glm::uvec2& size, const glm::uvec2& origin) noexcept
        : size(size), origin(origin)
    {
    }

    Viewport::Viewport(const glm::uvec4& values) noexcept
        : origin(values[0], values[1]), size(values[2], values[3])
    {
    }

    Viewport::Viewport(glm::uint x, glm::uint y, glm::uint w, glm::uint h) noexcept
        : origin(x, y), size(w, h)
    {
    }

    glm::uvec4 Viewport::getValues() const noexcept
    {
        return glm::ivec4(origin.x, origin.y, size.x, size.y);
    }

    glm::vec2 Viewport::screenToViewportPoint(const glm::vec2& point) const noexcept
    {
        return (point - glm::vec2(origin)) / glm::vec2(size);
    }

    glm::vec2 Viewport::viewportToScreenPoint(const glm::vec2& point) const noexcept
    {
        return glm::vec2(origin) + (point * glm::vec2(size));
    }

    glm::vec2 Viewport::adaptFromScreenPoint(const glm::vec2& point, const glm::vec2& size) const noexcept
    {
        return (point - glm::vec2(origin)) * glm::vec2(size) / size;
    }

    void Viewport::bgfxSetup(bgfx::ViewId viewId) const noexcept
    {
        bgfx::setViewRect(viewId, origin.x, origin.y, size.x, size.y);
    }
}