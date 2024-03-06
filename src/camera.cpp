#include <darmok/camera.hpp>
#include <bx/math.h>
#include <glm/gtc/type_ptr.hpp>

namespace darmok
{
    Camera::Camera(const glm::mat4& matrix, bgfx::ViewId viewId)
        : _matrix(matrix)
        , _viewId(viewId)
    {
    }

    const glm::mat4& Camera::getMatrix() const
    {
        return _matrix;
    }

    Camera& Camera::setMatrix(const glm::mat4& matrix)
    {
        _matrix = matrix;
        return *this;
    }

    Camera& Camera::setProjection(float fovy, float aspect, float near, float far)
    {
        bx::mtxProj(glm::value_ptr(_matrix), fovy, aspect, near, far, bgfx::getCaps()->homogeneousDepth);
        return *this;
    }

    Camera& Camera::setProjection(float fovy, float aspect, float near)
    {
        bx::mtxProjInf(glm::value_ptr(_matrix), glm::radians(fovy), aspect, near, bgfx::getCaps()->homogeneousDepth);
        return *this;
    }

    Camera& Camera::setOrtho(float left, float right, float bottom, float top, float near, float far, float offset)
    {
        bx::mtxOrtho(glm::value_ptr(_matrix), left, right, bottom, top, near, far, offset, bgfx::getCaps()->homogeneousDepth);
        return *this;
    }

    Camera& Camera::setViewId(bgfx::ViewId viewId)
    {
        _viewId = viewId;
        return *this;
    }

    bgfx::ViewId Camera::getViewId() const
    {
        return _viewId;
    }

    Camera& Camera::setEntityFilter(std::unique_ptr<IEntityFilter>&& filter)
    {
        _entityFilter = std::move(filter);
        return *this;
    }

    OptionalRef<const IEntityFilter> Camera::getEntityFilter() const
    {
        if (_entityFilter)
        {
            return *_entityFilter;
        }
        return nullptr;
    }

    OptionalRef<IEntityFilter> Camera::getEntityFilter()
    {
        if (_entityFilter)
        {
            return *_entityFilter;
        }
        return nullptr;
    }

    ViewRect::ViewRect(const ViewVec& size, const ViewVec& origin)
        : _size(size), _origin(origin)
    {
    }

    void ViewRect::setSize(const ViewVec& size)
    {
        _size = size;
    }

    void ViewRect::setOrigin(const ViewVec& origin)
    {
        _origin = origin;
    }

    const ViewVec& ViewRect::getSize() const
    {
        return _size;
    }

    const ViewVec& ViewRect::getOrigin() const
    {
        return _origin;
    }

    void ViewRect::bgfxConfig(bgfx::ViewId viewId) const
    {
        bgfx::setViewRect(viewId, _origin.x, _origin.y, _size.x, _size.y);
    }

}