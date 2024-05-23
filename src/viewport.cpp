#include <darmok/viewport.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <bx/math.h>

namespace darmok
{
    const glm::ivec4 Viewport::standardValues(0, 0, 1, 1);

    Viewport::Viewport(const glm::uvec2& size) noexcept
        : values(0, 0, size)
    {
    }

    Viewport::Viewport(const glm::ivec4& values) noexcept
        : values(values)
    {
    }

    Viewport::Viewport(int x, int y, int w, int h) noexcept
        : values(x, y, w, h)
    {
    }

    glm::uvec2 Viewport::getSize() const noexcept
    {
        return glm::uvec2(values[2], values[3]);
    }

    glm::ivec2 Viewport::getOrigin() const noexcept
    {
        return glm::ivec2(values[0], values[1]);
    }

    void Viewport::setSize(const glm::uvec2& size) noexcept
    {
        values[2] = size.x;
        values[3] = size.y;
    }

    void Viewport::setOrigin(const glm::ivec2& origin) noexcept
    {
        values[0] = origin.x;
        values[1] = origin.y;
    }

    glm::vec2 Viewport::screenToViewportPoint(const glm::vec2& point) const noexcept
    {
        return (point - glm::vec2(getOrigin())) / glm::vec2(getSize());
    }

    glm::vec2 Viewport::viewportToScreenPoint(const glm::vec2& point) const noexcept
    {
        return glm::vec2(getOrigin()) + (point * glm::vec2(getSize()));
    }

    glm::mat4 Viewport::getOrtho(float near, float far) const noexcept
    {
        glm::mat4 proj(1);
        auto origin = getOrigin();
        auto size = getSize();
        bx::mtxOrtho(glm::value_ptr(proj), origin.x, size.x, size.y, origin.y, near, far, 0.F, bgfx::getCaps()->homogeneousDepth);
        return proj;
    }

    glm::vec2 Viewport::adaptFromScreenPoint(const glm::vec2& point, const glm::vec2& size) const noexcept
    {
        return (point - glm::vec2(getOrigin())) * glm::vec2(getSize()) / size;
    }

    void Viewport::bgfxSetup(bgfx::ViewId viewId) const noexcept
    {
        bgfx::setViewRect(viewId, values[0], values[1], values[2], values[3]);
    }
}