#include <darmok/camera.hpp>
#include <darmok/transform.hpp>
#include <darmok/window.hpp>
#include <darmok/shape.hpp>
#include <darmok/texture.hpp>
#include <darmok/scene.hpp>
#include <darmok/app.hpp>
#include <darmok/math.hpp>
#include <darmok/render_scene.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <bx/math.h>

namespace darmok
{
    Camera::Camera(const glm::mat4& projMatrix) noexcept
        : _proj(projMatrix)
        , _enabled(true)
    {
    }

    Camera::~Camera()
    {
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
        if (_enabled != enabled)
        {
            _enabled = enabled;
            if (!enabled)
            {
                _scene->getRenderGraph().removeNode(_renderGraph.id());
            }
        }
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

    RenderChain& Camera::getRenderChain() noexcept
    {
        return _renderChain;
    }

    const RenderChain& Camera::getRenderChain() const noexcept
    {
        return _renderChain;
    }

    const glm::mat4& Camera::getProjectionMatrix() const noexcept
    {
        return _proj;
    }

    Camera& Camera::setProjectionMatrix(const glm::mat4& matrix) noexcept
    {
        _proj = matrix;
        _winProj.reset();
        return *this;
    }

    Camera& Camera::setPerspective(float fovy, float aspect, float near) noexcept
    {
        auto proj = Math::perspective(glm::radians(fovy), aspect, near);
        setProjectionMatrix(proj);
        return *this;
    }

    Camera& Camera::setPerspective(float fovy, float aspect, float near, float far) noexcept
    {
        auto proj = Math::perspective(glm::radians(fovy), aspect, near, far);
        setProjectionMatrix(proj);
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
        auto proj = vp.ortho(center, near, far);
        setProjectionMatrix(proj);
        return *this;
    }

    Camera& Camera::setOrtho(const glm::uvec2& size, const glm::vec2& center, float near, float far) noexcept
    {
        return setOrtho(Viewport(size), center, near, far);
    }

    Camera& Camera::setWindowPerspective(float fovy, float near, float far) noexcept
    {
        _winProj = WindowPerspectiveData(fovy, near, far);
        if (_app)
        {
            updateWindowProjection();
        }
        return *this;
    }

    Camera& Camera::setWindowOrtho(const glm::vec2& center, float near, float far) noexcept
    {
        _winProj = WindowOrthoData(center, near, far);
        if (_app)
        {
            updateWindowProjection();
        }
        return *this;
    }

    bool Camera::updateWindowProjection() noexcept
    {
        if (!_app || !_winProj)
        {
            return false;
        }
        auto& size = _app->getWindow().getPixelSize();
        auto ptr = &_winProj.value();
        if (auto winPersp = std::get_if<WindowPerspectiveData>(ptr))
        {
            float aspect = (float)size.x / size.y;
            _proj = Math::perspective(glm::radians(winPersp->fovy), aspect, winPersp->near, winPersp->far);
            return true;
        }
        else if (auto winOrtho = std::get_if<WindowOrthoData>(ptr))
        {
            _proj = Viewport(size).ortho(winOrtho->center, winOrtho->near, winOrtho->far);
            return true;
        }
        return false;
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
        auto rgSuffix = std::to_string(scene.getEntity(*this));
        _renderGraph.clear();
        _renderGraph.setName("Camera " + rgSuffix);
        _renderChain.init(_renderGraph, scene.getRenderChain());

        if (_entityFilter != nullptr)
        {
            _entityFilter->init(scene.getRegistry());
        }
        for(auto& renderer : _renderers)
        {
            renderer->init(*this, scene, app);
        }
        updateRenderGraph();
    }

    void Camera::renderReset()
    {
        _renderGraph.clear();
        for (auto& renderer : _renderers)
        {
            renderer->renderReset();
        }
        updateWindowProjection();
        _renderChain.renderReset();
    }

    void Camera::updateRenderGraph() noexcept
    {
        if (_scene && _enabled)
        {
            _scene->getRenderGraph().setChild(_renderGraph);
        }
    }

    void Camera::shutdown()
    {
        for (auto itr = _renderers.rbegin(); itr != _renderers.rend(); ++itr)
        {
            (*itr)->shutdown();
        }
        _renderChain.shutdown();
        _renderGraph.clear();
    }

    void Camera::update(float deltaTime)
    {
        for (auto& renderer : _renderers)
        {
            renderer->update(deltaTime);
        }
        updateRenderGraph();
        _renderChain.setViewport(getCurrentViewport());
    }

    void Camera::configureView(bgfx::ViewId viewId) const noexcept
    {
        auto renderTex = _renderChain.getFirstTexture();
        _renderChain.configureView(viewId, renderTex);
    }

    void Camera::beforeRenderView(bgfx::ViewId viewId) const noexcept
    {
        auto projPtr = glm::value_ptr(_proj);
        const void* viewPtr = nullptr;
        if (auto trans = getTransform())
        {
            viewPtr = glm::value_ptr(trans->getWorldInverse());
        }
        bgfx::setViewTransform(viewId, viewPtr, projPtr);
    }

    void Camera::beforeRenderEntity(Entity entity, bgfx::Encoder& encoder) const noexcept
    {
        const void* transMtx = nullptr;
        if (_scene)
        {
            if (auto trans = _scene->getComponent<const Transform>(entity))
            {
                transMtx = glm::value_ptr(trans->getWorldMatrix());
            }
        }
        if (transMtx)
        {
            encoder.setTransform(transMtx);
        }
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
        ;
        auto entity = _scene->getEntity(*this);
        if (entity == entt::null)
        {
            return nullptr;
        }
        return _scene->getComponent<Transform>(entity);
    }

    glm::mat4 Camera::getModelMatrix() const noexcept
    {
        if (auto trans = getTransform())
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