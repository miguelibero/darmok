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
        , _renderChain(*this)
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
            if (!enabled && _scene)
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
        _vpProj.reset();
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

    Camera& Camera::setViewportPerspective(float fovy, float near, float far) noexcept
    {
        _vpProj = PerspectiveData(fovy, near, far);
        if (_app)
        {
            updateViewportProjection();
        }
        return *this;
    }

    Camera& Camera::setViewportOrtho(const glm::vec2& center, float near, float far) noexcept
    {
        _vpProj = OrthoData(center, near, far);
        if (_app)
        {
            updateViewportProjection();
        }
        return *this;
    }

    bool Camera::updateViewportProjection() noexcept
    {
        if (!_app || !_vpProj)
        {
            return false;
        }
        auto vp = getCurrentViewport();
        auto ptr = &_vpProj.value();
        if (auto winPersp = std::get_if<PerspectiveData>(ptr))
        {
            float aspect = vp.getAspectRatio();
            _proj = Math::perspective(glm::radians(winPersp->fovy), aspect, winPersp->near, winPersp->far);
            return true;
        }
        else if (auto winOrtho = std::get_if<OrthoData>(ptr))
        {
            _proj = vp.ortho(winOrtho->center, winOrtho->near, winOrtho->far);
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


    Camera::ComponentRefs Camera::copyComponentContainer() const noexcept
    {
        ComponentRefs refs;
        for (auto& [type, component] : _components)
        {
            refs.emplace_back(*component);
        }
        return refs;
    }

    void Camera::init(Scene& scene, App& app)
    {
        _scene = scene;
        _app = app;
        auto rgSuffix = std::to_string(scene.getEntity(*this));
        _renderGraph.clear();
        auto name = "Camera " + rgSuffix;
        _renderGraph.setName(name);
        _renderChain.init(name + " render chain", -RenderPassDefinition::kMaxPriority);

        if (_entityFilter != nullptr)
        {
            _entityFilter->init(scene.getRegistry());
        }
        for(auto& comp : copyComponentContainer())
        {
            comp.get().init(*this, scene, app);
        }
        updateRenderGraph();
    }

    void Camera::renderReset()
    {
        _renderGraph.clear();
        for (auto& comp : copyComponentContainer())
        {
            comp.get().renderReset();
        }
        updateViewportProjection();
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
        auto components = copyComponentContainer();
        for (auto itr = components.rbegin(); itr != components.rend(); ++itr)
        {
            itr->get().shutdown();
        }
        _renderChain.shutdown();
        _renderGraph.clear();
    }

    void Camera::update(float deltaTime)
    {
        for (auto& comp : copyComponentContainer())
        {
            comp.get().update(deltaTime);
        }
        _renderChain.update(deltaTime);
        updateRenderGraph();
        updateViewportProjection();
    }

    void Camera::configureView(bgfx::ViewId viewId) const
    {
        auto frameBuffer = _renderChain.getInput();
        _renderChain.configureView(viewId, frameBuffer);
    }

    void Camera::setViewTransform(bgfx::ViewId viewId) const noexcept
    {
        auto model = getModelMatrix();
        bgfx::setViewTransform(viewId, glm::value_ptr(model), glm::value_ptr(_proj));
    }

    void Camera::setEntityTransform(Entity entity, bgfx::Encoder& encoder) const noexcept
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

    void Camera::beforeRenderView(IRenderGraphContext& context) const noexcept
    {
        setViewTransform(context.getViewId());
        for (auto& comp : copyComponentContainer())
        {
            comp.get().beforeRenderView(context);
        }
    }

    void Camera::beforeRenderEntity(Entity entity, IRenderGraphContext& context) const noexcept
    {
        setEntityTransform(entity, context.getEncoder());
        for (auto& comp : copyComponentContainer())
        {
            comp.get().beforeRenderEntity(entity, context);
        }
    }

    Camera& Camera::addComponent(entt::id_type type, std::unique_ptr<ICameraComponent>&& renderer) noexcept
    {
        if (_scene)
        {
            renderer->init(*this, _scene.value(), _app.value());
        }
        _components.emplace_back(type, std::move(renderer));
        return *this;
    }

    Camera::Components::iterator Camera::findComponent(entt::id_type type) noexcept
    {
        return std::find_if(_components.begin(), _components.end(), [type](auto& elm) { return elm.first == type; });
    }

    Camera::Components::const_iterator Camera::findComponent(entt::id_type type) const noexcept
    {
        return std::find_if(_components.begin(), _components.end(), [type](auto& elm) { return elm.first == type; });
    }


    Camera::ComponentDependencies Camera::_compDeps;

    void Camera::registerComponentDependency(entt::id_type typeId1, entt::id_type typeId2)
    {
        if (typeId1 == typeId2)
        {
            throw std::invalid_argument("dependency loop");
        }
        _compDeps[typeId1].insert(typeId2);
        _compDeps[typeId2].insert(typeId1);
    }

    bool Camera::removeComponent(entt::id_type type) noexcept
    {
        auto itr = findComponent(type);
        auto found = itr != _components.end();
        if (found)
        {
            _components.erase(itr);
        }
        auto itr2 = _compDeps.find(type);
        if (itr2 != _compDeps.end())
        {
            for (auto& depTypeId : itr2->second)
            {
                if (removeComponent(depTypeId))
                {
                    found = true;
                }
            }
        }
        return found;
    }

    bool Camera::hasComponent(entt::id_type type) const noexcept
    {
        auto itr = findComponent(type);
        return itr != _components.end();
    }

    OptionalRef<ICameraComponent> Camera::getComponent(entt::id_type type) noexcept
    {
        auto itr = findComponent(type);
        if (itr == _components.end())
        {
            return nullptr;
        }
        return itr->second.get();
    }

    OptionalRef<const ICameraComponent> Camera::getComponent(entt::id_type type) const noexcept
    {
        auto itr = findComponent(type);
        if (itr == _components.end())
        {
            return nullptr;
        }
        return itr->second.get();
    }

    Camera& Camera::setViewport(const std::optional<Viewport>& viewport) noexcept
    {
        _viewport = viewport;
        updateViewportProjection();
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
        else if (_scene)
        {
            return _scene->getCurrentViewport();
        }
        else if (_app)
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

    glm::mat4 Camera::getModelInverse() const noexcept
    {
        if (auto trans = getTransform())
        {
            return trans->getWorldMatrix();
        }
        return glm::mat4(1);
    }

    Ray Camera::screenPointToRay(const glm::vec3& point) const noexcept
    {
        return Ray::unproject(point, getModelMatrix(), _proj, getCurrentViewport());
    }

    Ray Camera::viewportPointToRay(const glm::vec3& point) const noexcept
    {
        return Ray::unproject(point, getModelMatrix(), _proj);
    }

    glm::vec3 Camera::worldToScreenPoint(const glm::vec3& point) const noexcept
    {
        return glm::project(point, getModelMatrix(), _proj, getCurrentViewport().getValues());
    }

    glm::vec3 Camera::worldToViewportPoint(const glm::vec3& point) const noexcept
    {
        return glm::project(point, getModelMatrix(), _proj, Viewport().getValues());
    }

    glm::vec3 Camera::screenToWorldPoint(const glm::vec3& point) const noexcept
    {
        return glm::unProject(point, getModelMatrix(), _proj, getCurrentViewport().getValues());
    }

    glm::vec3 Camera::viewportToWorldPoint(const glm::vec3& point) const noexcept
    {
        return glm::unProject(point, getModelMatrix(), _proj, Viewport().getValues());
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

    Viewport Camera::getRenderChainViewport() const noexcept
    {
        return getCurrentViewport();
    }

    OptionalRef<RenderChain> Camera::getRenderChainParent() const noexcept
    {
        if (_scene)
        {
            return _scene->getRenderChain();
        }
        return nullptr;
    }

    RenderGraphDefinition& Camera::getRenderChainParentGraph() noexcept
    {
        return _renderGraph;
    }

    void Camera::onRenderChainInputChanged() noexcept
    {
        renderReset();
    }
}