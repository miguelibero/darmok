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
    Camera::Camera(const glm::mat4& matrix) noexcept
        : _matrix(matrix)
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

    Camera& Camera::setOrtho(const glm::vec4& edges, const glm::vec2& range, float offset) noexcept
    {
        bx::mtxOrtho(glm::value_ptr(_matrix), edges[0], edges[1], edges[2], edges[3], range[0], range[1], offset, bgfx::getCaps()->homogeneousDepth);
        return *this;
    }

    Camera& Camera::setOrtho(const glm::uvec2& size, const glm::vec2& range, float offset) noexcept
    {
        return setOrtho(glm::vec4(0, size.x, 0, size.y), range, offset);
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

    const std::vector<std::shared_ptr<Texture>>& Camera::getTargetTextures() noexcept
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
        std::optional<glm::uvec2> size;
        if (!_targetTextures.empty())
        {
            size = _targetTextures[0]->getSize();
        }
        else if (_app)
        {
            size = _app->getWindow().getPixelSize();
        }

        static const uint16_t clearFlags = BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;
        bgfx::setViewClear(viewId, clearFlags, 1.F, 0U);
        
        bgfx::setViewFrameBuffer(viewId, _frameBuffer);
        if (size)
        {
            bgfx::setViewRect(viewId, 0, 0, size->x, size->y);
        }

        // this dummy draw call is here to make sure that view is cleared
        // if no other draw calls are submitted to view.
        bgfx::touch(viewId);

        if (_scene)
        {
            auto projPtr = glm::value_ptr(_matrix);
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

            auto viewRect = registry.try_get<const ViewRect>(entity);
            if (viewRect != nullptr)
            {
                viewRect->beforeRenderView(encoder, viewId);
            }
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

    std::optional<Ray> Camera::screenPointToRay(const glm::vec2& point) const noexcept
    {
        if (!_scene || !_app)
        {
            return std::nullopt;
        }
        auto& registry = _scene->getRegistry();
        auto entity = entt::to_entity(registry.storage<Camera>(), *this);
        if (entity == entt::null)
        {
            return std::nullopt;
        }
        glm::mat4 model;
        auto trans = registry.try_get<const Transform>(entity);
        if (trans != nullptr)
        {
            model = trans->getWorldInverse();
        }
        auto viewport = glm::ivec4(0, 0, _app->getWindow().getPixelSize());
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

    void ViewRect::beforeRenderView(bgfx::Encoder& encoder, bgfx::ViewId viewId) const noexcept
    {
        bgfx::setViewRect(viewId, _viewport[0], _viewport[1], _viewport[2], _viewport[3]);
    }

}