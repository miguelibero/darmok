#include <darmok/camera.hpp>
#include <darmok/transform.hpp>
#include <darmok/window.hpp>
#include <darmok/shape.hpp>
#include <darmok/texture.hpp>
#include <darmok/scene.hpp>
#include <darmok/app.hpp>
#include <darmok/math.hpp>
#include <darmok/string.hpp>
#include <darmok/render_scene.hpp>

#include "camera.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <bx/math.h>

namespace darmok
{
    CameraImpl::CameraImpl(Camera& cam, const glm::mat4& projMatrix) noexcept
        : _cam(cam)
        , _proj(projMatrix)
        , _projInv(glm::inverse(projMatrix))
        , _enabled(true)
        , _renderChain(*this)
    {
    }

    const std::string& CameraImpl::getName() const noexcept
    {
        return _name;
    }

    void CameraImpl::setName(const std::string& name) noexcept
    {
        _name = name;
    }

    Scene& CameraImpl::getScene()
    {
        return _scene.value();
    }

    const Scene& CameraImpl::getScene() const
    {
        return _scene.value();
    }

    bool CameraImpl::isEnabled() const noexcept
    {
        if (_updateEnabled)
        {
            return _updateEnabled.value();
        }
        return _enabled;
    }

    void CameraImpl::setEnabled(bool enabled) noexcept
    {
        _updateEnabled = enabled;
    }

    RenderChain& CameraImpl::getRenderChain() noexcept
    {
        return _renderChain;
    }

    const RenderChain& CameraImpl::getRenderChain() const noexcept
    {
        return _renderChain;
    }

    const glm::mat4& CameraImpl::getProjectionMatrix() const noexcept
    {
        return _proj;
    }

    const glm::mat4& CameraImpl::getProjectionInverse() const noexcept
    {
        return _projInv;
    }

    void CameraImpl::setProjectionMatrix(const glm::mat4& matrix) noexcept
    {
        doSetProjectionMatrix(matrix);
        _vpProj.reset();
    }

    void CameraImpl::setPerspective(float fovy, float aspect, float near) noexcept
    {
        auto proj = Math::perspective(glm::radians(fovy), aspect, near);
        setProjectionMatrix(proj);
    }

    void CameraImpl::setPerspective(float fovy, float aspect, float near, float far) noexcept
    {
        auto proj = Math::perspective(glm::radians(fovy), aspect, near, far);
        setProjectionMatrix(proj);
    }

    void CameraImpl::setPerspective(float fovy, const glm::uvec2& size, float near) noexcept
    {
        float aspect = (float)size.x / size.y;
        return setPerspective(fovy, aspect, near);
    }

    void CameraImpl::setPerspective(float fovy, const glm::uvec2& size, float near, float far) noexcept
    {
        float aspect = (float)size.x / size.y;
        return setPerspective(fovy, aspect, near, far);
    }

    void CameraImpl::setOrtho(const Viewport& vp, const glm::vec2& center, float near, float far) noexcept
    {
        auto proj = vp.ortho(center, near, far);
        setProjectionMatrix(proj);
    }

    void CameraImpl::setOrtho(const glm::uvec2& size, const glm::vec2& center, float near, float far) noexcept
    {
        return setOrtho(Viewport(size), center, near, far);
    }

    void CameraImpl::setViewportPerspective(float fovy, float near, float far) noexcept
    {
        _vpProj = PerspectiveData(fovy, near, far);
        if (_app)
        {
            updateViewportProjection();
        }
    }

    void CameraImpl::setViewportOrtho(const glm::vec2& center, float near, float far) noexcept
    {
        _vpProj = OrthoData(center, near, far);
        if (_app)
        {
            updateViewportProjection();
        }
    }

    bool CameraImpl::updateViewportProjection() noexcept
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
            auto proj = Math::perspective(glm::radians(winPersp->fovy), aspect, winPersp->near, winPersp->far);
            doSetProjectionMatrix(proj);
            return true;
        }
        else if (auto winOrtho = std::get_if<OrthoData>(ptr))
        {
            auto proj = vp.ortho(winOrtho->center, winOrtho->near, winOrtho->far);
            doSetProjectionMatrix(proj);
            return true;
        }
        return false;
    }

    void CameraImpl::doSetProjectionMatrix(const glm::mat4& matrix) noexcept
    {
        _proj = matrix;
        _projInv = glm::inverse(matrix);
    }

    void CameraImpl::setCullingFilter(const EntityFilter& filter) noexcept
    {
        _cullingFilter = filter;
    }

    const EntityFilter& CameraImpl::getCullingFilter() const noexcept
    {
        return _cullingFilter;
    }

    void CameraImpl::init(Scene& scene, App& app)
    {
        _scene = scene;
        _app = app;

        for(auto comp : Components(_components))
        {
            comp->init(_cam, scene, app);
        }
    }

    bgfx::ViewId CameraImpl::renderReset(bgfx::ViewId viewId)
    {
        updateViewportProjection();
        _renderChain.beforeRenderReset();
        for (auto comp : Components(_components))
        {
            viewId = comp->renderReset(viewId);
        }
        viewId = _renderChain.renderReset(viewId);
        return viewId;
    }

    void CameraImpl::render()
    {
        if (!_enabled)
        {
            return;
        }
        for (auto comp : Components(_components))
        {
            comp->render();
        }
    }

    void CameraImpl::shutdown()
    {
        {
            auto components = Components(_components);
            for (auto itr = components.rbegin(); itr != components.rend(); ++itr)
            {
                (*itr)->shutdown();
            }
        }
        _renderChain.shutdown();
    }

    void CameraImpl::update(float deltaTime)
    {
        if (_updateEnabled)
        {
            _enabled = _updateEnabled.value();
            _updateEnabled.reset();
        }
        for (auto comp : Components(_components))
        {
            comp->update(deltaTime);
        }
        _renderChain.update(deltaTime);
        updateViewportProjection();
    }

    void CameraImpl::configureView(bgfx::ViewId viewId) const
    {
        auto frameBuffer = _renderChain.getInput();
        _renderChain.configureView(viewId, frameBuffer);
    }

    void CameraImpl::setViewTransform(bgfx::ViewId viewId) const noexcept
    {
        auto model = getModelMatrix();
        bgfx::setViewTransform(viewId, glm::value_ptr(model), glm::value_ptr(_proj));
    }

    void CameraImpl::setEntityTransform(Entity entity, bgfx::Encoder& encoder) const noexcept
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

    void CameraImpl::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) const noexcept
    {
        setViewTransform(viewId);
        for (auto comp : Components(_components))
        {
            comp->beforeRenderView(viewId, encoder);
        }
    }

    bool CameraImpl::shouldEntityBeCulled(Entity entity) const noexcept
    {
        for (auto comp : Components(_components))
        {
            if (comp->shouldEntityBeCulled(entity))
            {
                return true;
            }
        }
        return false;
    }

    void CameraImpl::beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) const noexcept
    {
        setEntityTransform(entity, encoder);
        for (auto comp : Components(_components))
        {
            comp->beforeRenderEntity(entity, viewId, encoder);
        }
    }

    void CameraImpl::addComponent(std::unique_ptr<ICameraComponent>&& component) noexcept
    {
        if (auto type = component->getCameraComponentType())
        {
            removeComponent(type);
        }
        if (_scene)
        {
            component->init(_cam, _scene.value(), _app.value());
        }
        _components.emplace_back(std::move(component));
    }

    CameraImpl::Components::iterator CameraImpl::findComponent(entt::id_type type) noexcept
    {
        return std::find_if(_components.begin(), _components.end(),
            [type](auto& comp) { return comp->getCameraComponentType() == type; });
    }

    CameraImpl::Components::const_iterator CameraImpl::findComponent(entt::id_type type) const noexcept
    {
        return std::find_if(_components.begin(), _components.end(),
            [type](auto& comp) { return comp->getCameraComponentType() == type; });
    }


    CameraImpl::ComponentDependencies CameraImpl::_compDeps;

    void CameraImpl::registerComponentDependency(entt::id_type typeId1, entt::id_type typeId2)
    {
        if (typeId1 == typeId2)
        {
            throw std::invalid_argument("dependency loop");
        }
        _compDeps[typeId1].insert(typeId2);
        _compDeps[typeId2].insert(typeId1);
    }

    bool CameraImpl::removeComponent(entt::id_type type) noexcept
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

    bool CameraImpl::hasComponent(entt::id_type type) const noexcept
    {
        auto itr = findComponent(type);
        return itr != _components.end();
    }

    OptionalRef<ICameraComponent> CameraImpl::getComponent(entt::id_type type) noexcept
    {
        auto itr = findComponent(type);
        if (itr == _components.end())
        {
            return nullptr;
        }
        return **itr;
    }

    OptionalRef<const ICameraComponent> CameraImpl::getComponent(entt::id_type type) const noexcept
    {
        auto itr = findComponent(type);
        if (itr == _components.end())
        {
            return nullptr;
        }
        return **itr;
    }

    void CameraImpl::setViewport(const std::optional<Viewport>& viewport) noexcept
    {
        if(_viewport == viewport)
        {
            return;
        }
        _viewport = viewport;
        updateViewportProjection();
    }

    const std::optional<Viewport>& CameraImpl::getViewport() const noexcept
    {
        return _viewport;
    }

    Viewport CameraImpl::getCurrentViewport() const noexcept
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

    OptionalRef<Transform> CameraImpl::getTransform() const noexcept
    {
        if (!_scene)
        {
            return nullptr;
        }
        ;
        auto entity = _scene->getEntity(_cam);
        if (entity == entt::null)
        {
            return nullptr;
        }
        return _scene->getComponent<Transform>(entity);
    }

    glm::mat4 CameraImpl::getModelMatrix() const noexcept
    {
        if (auto trans = getTransform())
        {
            return trans->getWorldInverse();
        }
        return glm::mat4(1);
    }

    glm::mat4 CameraImpl::getModelInverse() const noexcept
    {
        if (auto trans = getTransform())
        {
            return trans->getWorldMatrix();
        }
        return glm::mat4(1);
    }

    glm::mat4 CameraImpl::getScreenModelMatrix() const noexcept
    {
        // y = -y since screen origin is the top-left corner
        static const glm::vec3 invy(1.F, -1.F, 1.F);
        return glm::scale(glm::mat4(1.F), invy) * getModelMatrix();
    }

    Ray CameraImpl::screenPointToRay(const glm::vec3& point) const noexcept
    {
        return Ray::unproject(point, getScreenModelMatrix(), _proj, getCurrentViewport());
    }

    Ray CameraImpl::viewportPointToRay(const glm::vec3& point) const noexcept
    {
        return Ray::unproject(point, getScreenModelMatrix(), _proj);
    }

    glm::vec3 CameraImpl::worldToScreenPoint(const glm::vec3& point) const noexcept
    {
        return glm::project(point, getScreenModelMatrix(), _proj, getCurrentViewport().getValues());
    }

    glm::vec3 CameraImpl::worldToViewportPoint(const glm::vec3& point) const noexcept
    {
        return glm::project(point, getScreenModelMatrix(), _proj, Viewport().getValues());
    }

    glm::vec3 CameraImpl::screenToWorldPoint(const glm::vec3& point) const noexcept
    {
        return glm::unProject(point, getScreenModelMatrix(), _proj, getCurrentViewport().getValues());
    }

    glm::vec3 CameraImpl::viewportToWorldPoint(const glm::vec3& point) const noexcept
    {
        return glm::unProject(point, getScreenModelMatrix(), _proj, Viewport().getValues());
    }

    glm::vec3 CameraImpl::viewportToScreenPoint(const glm::vec3& point) const noexcept
    {
        auto z = point.z;
        auto p = getCurrentViewport().viewportToScreenPoint(point);
        return glm::vec3(p, z);
    }

    glm::vec3 CameraImpl::screenToViewportPoint(const glm::vec3& point) const noexcept
    {
        auto z = point.z;
        auto p = getCurrentViewport().screenToViewportPoint(point);
        return glm::vec3(p, z);
    }

    Viewport CameraImpl::getRenderChainViewport() const noexcept
    {
        return getCurrentViewport();
    }

    OptionalRef<RenderChain> CameraImpl::getRenderChainParent() const noexcept
    {
        if (_scene)
        {
            return _scene->getRenderChain();
        }
        return nullptr;
    }

    void CameraImpl::onRenderChainChanged() noexcept
    {
        if (_app)
        {
            _app->requestRenderReset();
        }
    }

    Camera::Camera(const glm::mat4& projMatrix) noexcept
        : _impl(std::make_unique<CameraImpl>(*this, projMatrix))
    {
    }

    Camera::~Camera() noexcept
    {
        // empty on purpose
    }

    CameraImpl& Camera::getImpl() noexcept
    {
        return *_impl;
    }

    const CameraImpl& Camera::getImpl() const noexcept
    {
        return *_impl;
    }

    Scene& Camera::getScene()
    {
        return _impl->getScene();
    }

    const Scene& Camera::getScene() const
    {
        return _impl->getScene();
    }

    const std::string& Camera::getName() const noexcept
    {
        return _impl->getName();
    }

    Camera& Camera::setName(const std::string& name) noexcept
    {
        _impl->setName(name);
        return *this;
    }

    const glm::mat4& Camera::getProjectionMatrix() const noexcept
    {
        return _impl->getProjectionMatrix();
    }

    const glm::mat4& Camera::getProjectionInverse() const noexcept
    {
        return _impl->getProjectionInverse();
    }

    Camera& Camera::setProjectionMatrix(const glm::mat4& matrix) noexcept
    {
        _impl->setProjectionMatrix(matrix);
        return *this;
    }

    Camera& Camera::setPerspective(float fovy, float aspect, float near) noexcept
    {
        _impl->setPerspective(fovy, aspect, near);
        return *this;
    }
    
    Camera& Camera::setPerspective(float fovy, float aspect, float near, float far) noexcept
    {
        _impl->setPerspective(fovy, aspect, near, far);
        return *this;
    }

    Camera& Camera::setPerspective(float fovy, const glm::uvec2& size, float near) noexcept
    {
        _impl->setPerspective(fovy, size, near);
        return *this;
    }

    Camera& Camera::setPerspective(float fovy, const glm::uvec2& size, float near, float far) noexcept
    {
        _impl->setPerspective(fovy, size, near, far);
        return *this;
    }
    
    Camera& Camera::setOrtho(const Viewport& viewport, const glm::vec2& center, float near, float far) noexcept
    {
        _impl->setOrtho(viewport, center, near, far);
        return *this;
    }

    Camera& Camera::setOrtho(const glm::uvec2& size, const glm::vec2& center, float near, float far) noexcept
    {
        _impl->setOrtho(size, center, near, far);
        return *this;
    }

    Camera& Camera::setViewportPerspective(float fovy, float near, float far) noexcept
    {
        _impl->setViewportPerspective(fovy, near, far);
        return *this;
    }

    Camera& Camera::setViewportOrtho(const glm::vec2& center, float near, float far) noexcept
    {
        _impl->setViewportOrtho(center, near, far);
        return *this;
    }

    Camera& Camera::setViewport(const std::optional<Viewport>& viewport) noexcept
    {
        _impl->setViewport(viewport);
        return *this;
    }

    const std::optional<Viewport>& Camera::getViewport() const noexcept
    {
        return _impl->getViewport();
    }

    Viewport Camera::getCurrentViewport() const noexcept
    {
        return _impl->getCurrentViewport();
    }

    bool Camera::isEnabled() const noexcept
    {
        return _impl->isEnabled();
    }

    Camera& Camera::setEnabled(bool enabled) noexcept
    {
        _impl->setEnabled(enabled);
        return *this;
    }

    OptionalRef<Transform> Camera::getTransform() const noexcept
    {
        return _impl->getTransform();
    }

    glm::mat4 Camera::getModelMatrix() const noexcept
    {
        return _impl->getModelMatrix();
    }

    glm::mat4 Camera::getModelInverse() const noexcept
    {
        return _impl->getModelInverse();
    }

    Camera& Camera::setCullingFilter(const EntityFilter& filter) noexcept
    {
        _impl->setCullingFilter(filter);
        return *this;
    }

    const EntityFilter& Camera::getCullingFilter() const noexcept
    {
        return _impl->getCullingFilter();
    }

    Camera& Camera::addComponent(std::unique_ptr<ICameraComponent>&& comp) noexcept
    {
        _impl->addComponent(std::move(comp));
        return *this;
    }

    bool Camera::removeComponent(entt::id_type type) noexcept
    {
        return _impl->removeComponent(type);
    }

    bool Camera::hasComponent(entt::id_type type) const noexcept
    {
        return _impl->hasComponent(type);
    }

    OptionalRef<ICameraComponent> Camera::getComponent(entt::id_type type) noexcept
    {
        return _impl->getComponent(type);
    }

    OptionalRef<const ICameraComponent> Camera::getComponent(entt::id_type type) const noexcept
    {
        return _impl->getComponent(type);
    }

    Ray Camera::screenPointToRay(const glm::vec3& point) const noexcept
    {
        return _impl->screenPointToRay(point);
    }

    Ray Camera::viewportPointToRay(const glm::vec3& point) const noexcept
    {
        return _impl->viewportPointToRay(point);
    }

    glm::vec3 Camera::worldToScreenPoint(const glm::vec3& point) const noexcept
    {
        return _impl->worldToScreenPoint(point);
    }

    glm::vec3 Camera::worldToViewportPoint(const glm::vec3& point) const noexcept
    {
        return _impl->worldToViewportPoint(point);
    }

    glm::vec3 Camera::screenToWorldPoint(const glm::vec3& point) const noexcept
    {
        return _impl->screenToWorldPoint(point);
    }

    glm::vec3 Camera::viewportToWorldPoint(const glm::vec3& point) const noexcept
    {
        return _impl->viewportToWorldPoint(point);
    }

    glm::vec3 Camera::viewportToScreenPoint(const glm::vec3& point) const noexcept
    {
        return _impl->viewportToScreenPoint(point);
    }

    glm::vec3 Camera::screenToViewportPoint(const glm::vec3& point) const noexcept
    {
        return _impl->screenToViewportPoint(point);
    }

    RenderChain& Camera::getRenderChain() noexcept
    {
        return _impl->getRenderChain();
    }

    const RenderChain& Camera::getRenderChain() const noexcept
    {
        return _impl->getRenderChain();
    }
    
    void Camera::configureView(bgfx::ViewId viewId) const
    {
        _impl->configureView(viewId);
    }

    void Camera::setViewTransform(bgfx::ViewId viewId) const noexcept
    {
        _impl->setViewTransform(viewId);
    }

    void Camera::setEntityTransform(Entity entity, bgfx::Encoder& encoder) const noexcept
    {
        _impl->setEntityTransform(entity, encoder);
    }

    bool Camera::shouldEntityBeCulled(Entity entity) const noexcept
    {
        return _impl->shouldEntityBeCulled(entity);
    }

    void Camera::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) const noexcept
    {
        _impl->beforeRenderView(viewId, encoder);
    }

    void Camera::beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) const noexcept
    {
        _impl->beforeRenderEntity(entity, viewId, encoder);
    }

    void Camera::registerComponentDependency(entt::id_type typeId1, entt::id_type typeId2)
    {
        CameraImpl::registerComponentDependency(typeId1, typeId2);
    }
}