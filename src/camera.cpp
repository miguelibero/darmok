#include <darmok/camera.hpp>
#include <darmok/transform.hpp>
#include <darmok/window.hpp>
#include <darmok/shape.hpp>
#include <darmok/texture.hpp>
#include <darmok/scene.hpp>
#include <darmok/app.hpp>
#include <darmok/math.hpp>
#include <darmok/render.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <bx/math.h>

namespace darmok
{
    Camera::Camera(const glm::mat4& projMatrix) noexcept
        : _proj(projMatrix)
        , _frameBuffer{ bgfx::kInvalidHandle }
        , _enabled(true)
    {
    }

    Camera::~Camera()
    {
        if (isValid(_frameBuffer))
        {
            bgfx::destroy(_frameBuffer);
        }
    }

    const EntityRegistry& Camera::getRegistry() const
    {
        return _scene->getRegistry();
    }

    bool Camera::isEnabled() const noexcept
    {
        return _enabled;
    }

    Camera& Camera::setEnabled(bool enabled) noexcept
    {
        _enabled = enabled;
        return *this;
    }

    RenderGraphDefinition& Camera::getRenderGraph() noexcept
    {
        return _renderGraph;
    }

    const RenderGraphDefinition& Camera::getRenderGraph() const noexcept
    {
        return _renderGraph;
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

    Camera& Camera::setPerspective(float fovy, float aspect, float near) noexcept
    {
        _proj = Math::perspective(glm::radians(fovy), aspect, near);
        return *this;
    }

    Camera& Camera::setPerspective(float fovy, float aspect, float near, float far) noexcept
    {
        _proj = Math::perspective(glm::radians(fovy), aspect, near, far);
        return *this;
    }

    Camera& Camera::setPerspective(float fovy, const glm::uvec2& size, float near) noexcept
    {
        float aspect = (float)size.x / size.y;
        return setPerspective(fovy, aspect, near);
    }

    Camera& Camera::setPerspective(float fovy, const glm::uvec2& size, float near, float far) noexcept
    {
        float aspect = (float)size.x / size.y;
        return setPerspective(fovy, aspect, near, far);
    }

    Camera& Camera::setOrtho(const Viewport& vp, const glm::vec2& center, float near, float far) noexcept
    {
        _proj = vp.ortho(center, near, far);
        return *this;
    }

    Camera& Camera::setOrtho(const glm::uvec2& size, const glm::vec2& center, float near, float far) noexcept
    {
        return setOrtho(Viewport(size), center, near, far);
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
        if (_targetTextures == textures)
        {
            return *this;
        }
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
        _frameBuffer = bgfx::createFrameBuffer(uint8_t(handles.size()), &handles.front());
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
        _renderGraph.clear();
        _renderGraph.setName("Render Camera " + scene.getEntity(*this));
        if (_entityFilter != nullptr)
        {
            _entityFilter->init(scene.getRegistry());
        }
        for(auto& renderer : _renderers)
        {
            renderer->init(*this, scene, app);
        }
    }

    void Camera::shutdown()
    {
        for (auto& renderer : _renderers)
        {
            renderer->shutdown();
        }
    }

    void Camera::update(float deltaTime)
    {
        for (auto& renderer : _renderers)
        {
            renderer->update(deltaTime);
        }

        _app->getRenderGraph().setChild(_renderGraph);
    }

    void Camera::configureView(bgfx::ViewId viewId) const noexcept
    {
        bgfx::setViewFrameBuffer(viewId, _frameBuffer);
        getCurrentViewport().configureView(viewId);
    }

    void Camera::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) const noexcept
    {
        auto projPtr = glm::value_ptr(_proj);
        const void* viewPtr = nullptr;

        if (_model)
        {
            viewPtr = glm::value_ptr(_model.value());
        }
        else
        {
            auto trans = getTransform();
            if (trans != nullptr)
            {
                viewPtr = glm::value_ptr(trans->getWorldInverse());
            }
        }
        bgfx::setViewTransform(viewId, viewPtr, projPtr);
        encoder.touch(viewId);
    }

    void Camera::beforeRenderEntity(Entity entity, bgfx::Encoder& encoder) const noexcept
    {
        const void* transMtx = nullptr;
        if (_scene)
        {
            auto& registry = _scene->getRegistry();
            auto trans = registry.try_get<const Transform>(entity);
            if (trans != nullptr)
            {
                transMtx = glm::value_ptr(trans->getWorldMatrix());
            }
        }
        encoder.setTransform(transMtx);
    }

    Camera& Camera::addRenderer(std::unique_ptr<IRenderer>&& renderer) noexcept
    {
        if (_scene)
        {
            renderer->init(*this, _scene.value(), _app.value());
        }
        _renderers.push_back(std::move(renderer));
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
        if (_model)
        {
            return _model.value();
        }
        auto trans = getTransform();
        if (trans)
        {
            return trans->getWorldInverse();
        }
        return glm::mat4(1);
    }

    Camera& Camera::setModelMatrix(std::optional<glm::mat4> mat) noexcept
    {
        _model = mat;
        return *this;
    }

    Ray Camera::screenPointToRay(const glm::vec3& point) const noexcept
    {
        return Ray::unproject(point, getModelMatrix(), _proj, getCurrentViewport().getValues());
    }

    Ray Camera::viewportPointToRay(const glm::vec3& point) const noexcept
    {
        return Ray::unproject(point, getModelMatrix(), _proj, Viewport::getStandardValues());
    }

    glm::vec3 Camera::worldToScreenPoint(const glm::vec3& point) const noexcept
    {
        return glm::project(point, getModelMatrix(), _proj, getCurrentViewport().getValues());
    }

    glm::vec3 Camera::worldToViewportPoint(const glm::vec3& point) const noexcept
    {
        return glm::project(point, getModelMatrix(), _proj, Viewport::getStandardValues());
    }

    glm::vec3 Camera::screenToWorldPoint(const glm::vec3& point) const noexcept
    {
        return glm::unProject(point, getModelMatrix(), _proj, getCurrentViewport().getValues());
    }

    glm::vec3 Camera::viewportToWorldPoint(const glm::vec3& point) const noexcept
    {
        return glm::unProject(point, getModelMatrix(), _proj, Viewport::getStandardValues());
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