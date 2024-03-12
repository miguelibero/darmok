#include <darmok/camera.hpp>
#include <darmok/transform.hpp>
#include <bx/math.h>
#include <glm/gtc/type_ptr.hpp>

namespace darmok
{
    Camera::Camera(const glm::mat4& matrix) noexcept
        : _matrix(matrix)
    {
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

    Camera& Camera::setEntityFilter(std::unique_ptr<IEntityFilter>&& filter) noexcept
    {
        _entityFilter = std::move(filter);
        return *this;
    }

    void Camera::update(EntityRegistry& registry) noexcept
    {
        if (_entityFilter != nullptr)
        {
            _entityFilter->init(registry);
        }
    }

    EntityRuntimeView Camera::bgfxConfig(EntityRegistry& registry, bgfx::ViewId viewId) const noexcept
    {
        auto projPtr = glm::value_ptr(_matrix);

        auto entity = entt::to_entity(registry, *this);
        auto trans = registry.try_get<const Transform>(entity);
        const void* viewPtr = nullptr;
        if (trans != nullptr)
        {
            viewPtr = glm::value_ptr(trans->getInverse());
        }
        bgfx::setViewTransform(viewId, viewPtr, projPtr);

        auto viewRect = registry.try_get<const ViewRect>(entity);
        if (viewRect != nullptr)
        {
            viewRect->bgfxConfig(viewId);
        }

        EntityRuntimeView entities;
        if (_entityFilter != nullptr)
        {
            (*_entityFilter)(entities);
        }

        return entities;
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