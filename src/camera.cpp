#include <darmok/camera.hpp>
#include <darmok/transform.hpp>
#include <darmok/window.hpp>
#include <darmok/math.hpp>
#include <bx/math.h>
#include <glm/gtc/type_ptr.hpp>

namespace darmok
{
    Camera::Camera(const glm::mat4& matrix) noexcept
        : _matrix(matrix)
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

    Camera& Camera::setProjection(float fovy, float aspect, const glm::vec2& range) noexcept
    {
        bx::mtxProj(glm::value_ptr(_matrix), fovy, aspect, range[0], range[1], bgfx::getCaps()->homogeneousDepth);
        return *this;
    }

    Camera& Camera::setProjection(float fovy, float aspect, float near) noexcept
    {
        bx::mtxProjInf(glm::value_ptr(_matrix), glm::radians(fovy), aspect, near, bgfx::getCaps()->homogeneousDepth);
        return *this;
    }

    Camera& Camera::setWindowProjection(float fovy, const glm::vec2& range) noexcept
    {
        auto size = _app->getWindow().getPixelSize();
        float aspect = (float)size.x / size.y;
        return setProjection(fovy, aspect, range);
    }

    Camera& Camera::setWindowProjection(float fovy, float near) noexcept
    {
        auto size = _app->getWindow().getPixelSize();
        float aspect = (float)size.x / size.y;
        return setProjection(fovy, aspect, near);
    }

    Camera& Camera::setWindowOrtho(const glm::vec2& range, float offset) noexcept
    {
        auto& vp = _app->getWindow().getViewport();
        glm::vec4 edges(vp[0], vp[2], vp[3], vp[1]);
        return setOrtho(edges, range, offset);
    }

    Camera& Camera::setOrtho(const glm::vec4& edges, const glm::vec2& range, float offset) noexcept
    {
        bx::mtxOrtho(glm::value_ptr(_matrix), edges[0], edges[1], edges[2], edges[3], range[0], range[1], offset, bgfx::getCaps()->homogeneousDepth);
        return *this;
    }

    Camera& Camera::setEntityFilter(std::unique_ptr<IEntityFilter>&& filter) noexcept
    {
        _entityFilter = std::move(filter);
        if (_scene)
        {
            _entityFilter->init(_scene->getRegistry());
        }
        return *this;
    }

    void Camera::bgfxConfig(const EntityRegistry& registry, bgfx::ViewId viewId) const noexcept
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
    }

    void Camera::filterEntityView(EntityRuntimeView& entities) const noexcept
    {
        if (_entityFilter != nullptr)
        {
            (*_entityFilter)(entities);
        }
    }

    void Camera::init(Scene& scene, App& app)
    {
        _scene = scene;
        _app = app;
        if (_entityFilter != nullptr)
        {
            _entityFilter->init(scene.getRegistry());
        }
        if (_renderer != nullptr)
        {
            _renderer->init(*this, scene, app);
        }
        for(auto& component : _components)
        {
            component->init(*this, scene, app);
        }
    }

    void Camera::shutdown()
    {
        if (_renderer != nullptr)
        {
            _renderer->shutdown();
        }
        for (auto& component : _components)
        {
            component->shutdown();
        }
    }

    void Camera::update(float deltaTime)
    {
        if (_renderer != nullptr)
        {
            _renderer->update(deltaTime);
        }
        for (auto& component : _components)
        {
            component->update(deltaTime);
        }
    }

    bgfx::ViewId Camera::render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const
    {
        if (!_app || !_scene)
        {
            return viewId;
        }
        auto& win = _app->getWindow();
        auto& registry = _scene->getRegistry();
        win.bgfxConfig(viewId);
        bgfxConfig(registry, viewId);

        if(_renderer != nullptr)
        {
            viewId = _renderer->render(encoder, viewId);
        }

        return viewId;
    }

    Camera& Camera::setRenderer(std::unique_ptr<ICameraRenderer>&& renderer) noexcept
    {
        if (_scene)
        {
            renderer->init(*this, _scene.value(), _app.value());
        }
        _renderer = std::move(renderer);
        return *this;
    }

    Camera& Camera::addComponent(std::unique_ptr<ICameraComponent>&& component) noexcept
    {
        if (_scene)
        {
            component->init(*this, _scene.value(), _app.value());
        }
        _components.push_back(std::move(component));
        return *this;
    }

    std::optional<Ray> Camera::screenPointToRay(const glm::vec2& point) const noexcept
    {
        if (!_scene || !_app)
        {
            return std::nullopt;
        }
        glm::mat4 model;
        auto& registry = _scene->getRegistry();
        auto entity = entt::to_entity(registry, *this);
        auto trans = registry.try_get<const Transform>(entity);
        if (trans != nullptr)
        {
            model = trans->getInverse();
        }
        glm::ivec4 viewport = _app->getWindow().getViewport();
        return Ray::unproject(point, model, _matrix, viewport);
    }

    ViewRect::ViewRect(const glm::ivec4& viewport) noexcept
        : _viewport(viewport)
    {
    }

    const glm::ivec4& ViewRect::getViewport() const noexcept
    {
        return _viewport;
    }

    void ViewRect::setViewport(const glm::ivec4& viewport) noexcept
    {
        _viewport = viewport;
    }

    void ViewRect::bgfxConfig(bgfx::ViewId viewId) const noexcept
    {
        bgfx::setViewRect(viewId, _viewport[0], _viewport[1], _viewport[2], _viewport[3]);
    }

}