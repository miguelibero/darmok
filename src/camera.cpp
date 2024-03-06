#include <darmok/camera.hpp>
#include <bx/math.h>
#include <glm/gtc/type_ptr.hpp>

namespace darmok
{
    Camera::Camera(const glm::mat4& matrix, bgfx::ViewId viewId) noexcept
        : _matrix(matrix)
        , _viewId(viewId)
    {
    }

    const glm::mat4& Camera::getMatrix() const noexcept
    {
        return _matrix;
    }

    Camera& Camera::setMatrix(const glm::mat4& matrix) noexcept
    {
        _matrix = matrix;
        return *this;
    }

    Camera& Camera::setProjection(float fovy, float aspect, float near, float far) noexcept
    {
        bx::mtxProj(glm::value_ptr(_matrix), fovy, aspect, near, far, bgfx::getCaps()->homogeneousDepth);
        return *this;
    }

    Camera& Camera::setProjection(float fovy, float aspect, float near) noexcept
    {
        bx::mtxProjInf(glm::value_ptr(_matrix), glm::radians(fovy), aspect, near, bgfx::getCaps()->homogeneousDepth);
        return *this;
    }

    Camera& Camera::setOrtho(float left, float right, float bottom, float top, float near, float far, float offset) noexcept
    {
        bx::mtxOrtho(glm::value_ptr(_matrix), left, right, bottom, top, near, far, offset, bgfx::getCaps()->homogeneousDepth);
        return *this;
    }

    Camera& Camera::setViewId(bgfx::ViewId viewId) noexcept
    {
        _viewId = viewId;
        return *this;
    }

    bgfx::ViewId Camera::getViewId() const noexcept
    {
        return _viewId;
    }

    Camera& Camera::setEntityFilter(std::unique_ptr<IEntityFilter>&& filter) noexcept
    {
        _entityFilter = std::move(filter);
        return *this;
    }

    OptionalRef<const IEntityFilter> Camera::getEntityFilter() const noexcept
    {
        if (_entityFilter)
        {
            return *_entityFilter;
        }
        return nullptr;
    }

    OptionalRef<IEntityFilter> Camera::getEntityFilter() noexcept
    {
        if (_entityFilter)
        {
            return *_entityFilter;
        }
        return nullptr;
    }

    ViewRect::ViewRect(const ViewVec& size, const ViewVec& origin) noexcept
        : _size(size), _origin(origin)
    {
    }

    void ViewRect::setSize(const ViewVec& size) noexcept
    {
        _size = size;
    }

    void ViewRect::setOrigin(const ViewVec& origin) noexcept
    {
        _origin = origin;
    }

    const ViewVec& ViewRect::getSize() const noexcept
    {
        return _size;
    }

    const ViewVec& ViewRect::getOrigin() const noexcept
    {
        return _origin;
    }

    void ViewRect::bgfxConfig(bgfx::ViewId viewId) const noexcept
    {
        bgfx::setViewRect(viewId, _origin.x, _origin.y, _size.x, _size.y);
    }

}