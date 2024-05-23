#include <darmok/camera.hpp>
#include <darmok/transform.hpp>
#include <darmok/window.hpp>
#include <darmok/shape.hpp>
#include <darmok/texture.hpp>
#include <darmok/scene.hpp>
#include <darmok/app.hpp>
#include <bx/math.h>
#include <glm/gtc/type_ptr.hpp>

namespace darmok
{
    Camera::Camera(const glm::mat4& projMatrix) noexcept
        : _proj(projMatrix)
        , _frameBuffer{ bgfx::kInvalidHandle }
    {
    }

    Camera::~Camera()
    {
        if (isValid(_frameBuffer))
        {
            bgfx::destroy(_frameBuffer);
        }
    }

    const glm::mat4& Camera::getProjectionMatrix() const noexcept
    {
        return _proj;
    }

    Camera& Camera::setProjectionMatrix(const glm::mat4& matrix) noexcept
    {
        _proj = matrix;
        return *this;
    }

    Camera& Camera::setProjection(float fovy, float aspect, const glm::vec2& range) noexcept
    {
        bx::mtxProj(glm::value_ptr(_proj), fovy, aspect, range[0], range[1], bgfx::getCaps()->homogeneousDepth);
        return *this;
    }

    Camera& Camera::setProjection(float fovy, float aspect, float near) noexcept
    {
        bx::mtxProjInf(glm::value_ptr(_proj), glm::radians(fovy), aspect, near, bgfx::getCaps()->homogeneousDepth);
        return *this;
    }

    Camera& Camera::setProjection(float fovy, const glm::uvec2& size, const glm::vec2& range) noexcept
    {
        float aspect = (float)size.x / size.y;
        return setProjection(fovy, aspect, range);
    }

    Camera& Camera::setProjection(float fovy, const glm::uvec2& size, float near) noexcept
    {
        float aspect = (float)size.x / size.y;
        return setProjection(fovy, aspect, near);
    }

    Camera& Camera::setOrtho(const Viewport& vp, float near, float far) noexcept
    {
        auto bot = glm::vec2(vp.origin);
        auto top = bot + glm::vec2(vp.size);
        bx::mtxOrtho(glm::value_ptr(_proj), bot.x, top.x, bot.y, top.y, near, far, 0.F, bgfx::getCaps()->homogeneousDepth);
        return *this;
    }

    Camera& Camera::setOrtho(const glm::uvec2& size, float near, float far) noexcept
    {
        return setOrtho(Viewport(size), near, far);
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

    Camera& Camera::setTargetTextures(const std::vector<std::shared_ptr<Texture>>& textures) noexcept
    {
        if (_targetTextures != textures)
        {
            if (isValid(_frameBuffer))
            {
                bgfx::destroy(_frameBuffer);
            }
            _targetTextures = textures;
            std::vector<bgfx::TextureHandle> handles;
            handles.reserve(textures.size());
            for (auto& tex : textures)
            {
                handles.push_back(tex->getHandle());
            }
            _frameBuffer = bgfx::createFrameBuffer(handles.size(), &handles.front());
        }

        return *this;
    }

    const std::vector<std::shared_ptr<Texture>>& Camera::getTargetTextures() const noexcept
    {
        return _targetTextures;
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
        if(_renderer != nullptr)
        {
            viewId = _renderer->render(encoder, viewId);
        }
        return viewId;
    }

    void Camera::beforeRenderView(bgfx::Encoder& encoder, bgfx::ViewId viewId) const noexcept
    {
        static const uint16_t clearFlags = BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;
        bgfx::setViewClear(viewId, clearFlags, 1.F, 0U);
        
        bgfx::setViewFrameBuffer(viewId, _frameBuffer);

        getCurrentViewport().bgfxSetup(viewId);

        // this dummy draw call is here to make sure that view is cleared
        // if no other draw calls are submitted to view.
        bgfx::touch(viewId);

        if (_scene)
        {
            auto projPtr = glm::value_ptr(_proj);
            const void* viewPtr = nullptr;
            auto& registry = _scene->getRegistry();

            auto entity = entt::to_entity(registry.storage<Camera>(), *this);
            if (entity != entt::null)
            {
                auto trans = registry.try_get<const Transform>(entity);
                if (trans != nullptr)
                {
                    viewPtr = glm::value_ptr(trans->getWorldInverse());
                }
            }

            bgfx::setViewTransform(viewId, viewPtr, projPtr);
        }
        if (_renderer != nullptr)
        {
            _renderer->beforeRenderView(encoder, viewId);
        }
        for (auto& comp : _components)
        {
            comp->beforeRenderView(encoder, viewId);
        }
    }

    void Camera::beforeRenderEntity(Entity entity, bgfx::Encoder& encoder, bgfx::ViewId viewId) const noexcept
    {
        if (_renderer != nullptr)
        {
            _renderer->beforeRenderEntity(entity, encoder, viewId);
        }
        for (auto& comp : _components)
        {
            comp->beforeRenderEntity(entity, encoder, viewId);
        }
    }

    void Camera::afterRenderView(bgfx::Encoder& encoder, bgfx::ViewId viewId) const noexcept
    {
        if (_renderer != nullptr)
        {
            _renderer->afterRenderView(encoder, viewId);
        }
        for (auto& comp : _components)
        {
            comp->afterRenderView(encoder, viewId);
        }
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

    Camera& Camera::setViewport(const std::optional<Viewport>& viewport) noexcept
    {
        _viewport = viewport;
        return *this;
    }

    const std::optional<Viewport>& Camera::getViewport() const noexcept
    {
        return _viewport;
    }

    Viewport Camera::getCurrentViewport() const noexcept
    {
        if (_viewport)
        {
            return _viewport.value();
        }
        if (!_targetTextures.empty())
        {
            auto& size = _targetTextures[0]->getSize();
            return Viewport(0, 0, size.x, size.y);
        }
        if (_app)
        {
            return Viewport(_app->getWindow().getPixelSize());
        }
        return Viewport();
    }

    OptionalRef<Transform> Camera::getTransform() const noexcept
    {
        if (!_scene)
        {
            return nullptr;
        }
        auto& registry = _scene->getRegistry();
        auto entity = entt::to_entity(registry.storage<Camera>(), *this);
        if (entity == entt::null)
        {
            return nullptr;
        }
        return registry.try_get<Transform>(entity);
    }

    glm::mat4 Camera::getModelMatrix() const noexcept
    {
        auto trans = getTransform();
        if (trans)
        {
            return trans->getWorldInverse();
        }
        return glm::mat4(1);
    }

    Ray Camera::screenPointToRay(const glm::vec3& point) const noexcept
    {
        return Ray::unproject(point, getModelMatrix(), _proj, getCurrentViewport().getValues());
    }

    Ray Camera::viewportPointToRay(const glm::vec3& point) const noexcept
    {
        return Ray::unproject(point, getModelMatrix(), _proj, Viewport::standardValues);
    }

    glm::vec3 Camera::worldToScreenPoint(const glm::vec3& point) const noexcept
    {
        return glm::project(point, getModelMatrix(), _proj, getCurrentViewport().getValues());
    }

    glm::vec3 Camera::worldToViewportPoint(const glm::vec3& point) const noexcept
    {
        return glm::project(point, getModelMatrix(), _proj, Viewport::standardValues);
    }

    glm::vec3 Camera::screenToWorldPoint(const glm::vec3& point) const noexcept
    {
        return glm::unProject(point, getModelMatrix(), _proj, getCurrentViewport().getValues());
    }

    glm::vec3 Camera::viewportToWorldPoint(const glm::vec3& point) const noexcept
    {
        return glm::unProject(point, getModelMatrix(), _proj, Viewport::standardValues);
    }

    glm::vec3 Camera::viewportToScreenPoint(const glm::vec3& point) const noexcept
    {
        auto z = point.z;
        auto p = getCurrentViewport().viewportToScreenPoint(point);
        return glm::vec3(p, z);
    }

    glm::vec3 Camera::screenToViewportPoint(const glm::vec3& point) const noexcept
    {
        auto z = point.z;
        auto p = getCurrentViewport().screenToViewportPoint(point);
        return glm::vec3(p, z);
    }
}