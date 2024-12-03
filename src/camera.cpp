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
#include <darmok/camera_reflect.hpp>

#include "camera.hpp"
#include "scene.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <bx/math.h>

using namespace entt::literals;

namespace darmok
{
    CameraImpl::CameraImpl(Camera& cam, const glm::mat4& projMatrix) noexcept
        : _cam(cam)
        , _view(1.F)
        , _proj(projMatrix)
        , _projInv(glm::inverse(projMatrix))
        , _enabled(true)
        , _renderChain(*this)
        , _transformChanged(false)
        , _projData(CameraOrthoData{})
        , _viewport(0.F, 0.F, 1.F, 1.F)
    {
    }

    const std::string& CameraImpl::getName() const noexcept
    {
        if (!_name.empty())
        {
            return _name;
        }
        if (auto trans = getTransform())
        {
            return trans->getName();
        }
        return _name;
    }

    void CameraImpl::setName(const std::string& name) noexcept
    {
        _name = name;
    }

    entt::id_type CameraImpl::getId() const noexcept
    {
        return reinterpret_cast<uintptr_t>(static_cast<const void*>(this));
    }

    std::string CameraImpl::getDescName() const noexcept
    {
        if (!_name.empty())
        {
            return _name;
        }
        return StringUtils::binToHex(getId());
    }

    std::string CameraImpl::toString() const noexcept
    {
        return "Camera(" + getDescName() + ", " + glm::to_string(getProjectionMatrix()) + ")";
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

    glm::mat4 CameraImpl::getViewProjectionMatrix() const noexcept
    {
       return getProjectionMatrix() * getViewMatrix();
    }

    glm::mat4 CameraImpl::getViewProjectionInverse() const noexcept
    {
        return getViewInverse() * getProjectionInverse();
    }

    BoundingBox CameraImpl::getPlaneBounds(const Plane& plane) const noexcept
    {
        auto botLeftRay = viewportPointToRay(glm::vec3(0));
        auto dbl = botLeftRay.intersect(plane);
        auto botLeft = dbl ? botLeftRay * dbl.value() : glm::vec3(-bx::kFloatInfinity);
        auto topRightRay = viewportPointToRay(glm::vec3(1));
        auto dtr = topRightRay.intersect(plane);
        auto topRight = dtr ? topRightRay * dtr.value() : glm::vec3(bx::kFloatInfinity);

        auto centerRay = viewportPointToRay(glm::vec3(0.5));
        return { botLeft, topRight };
    }

    const CameraProjectionData& CameraImpl::getProjection() const noexcept
    {
        return _projData;
    }

    void CameraImpl::setProjection(const CameraProjectionData& data) noexcept
    {
        _projData = data;
        if (_app)
        {
            updateProjection();
        }
    }

    bool CameraImpl::updateProjection() noexcept
    {
        if (!_app)
        {
            return false;
        }
        auto vp = getCombinedViewport();
        auto ptr = &_projData;
        if (auto winPersp = std::get_if<CameraPerspectiveData>(ptr))
        {
            float aspect = vp.getAspectRatio();
            auto proj = Math::perspective(glm::radians(winPersp->fovy), aspect, winPersp->near, winPersp->far);
            setProjectionMatrix(proj);
            return true;
        }
        else if (auto winOrtho = std::get_if<CameraOrthoData>(ptr))
        {
            auto proj = vp.ortho(winOrtho->center, winOrtho->near, winOrtho->far);
            setProjectionMatrix(proj);
            return true;
        }
        return false;
    }

    void CameraImpl::setProjectionMatrix(const glm::mat4& matrix) noexcept
    {
        if (_proj == matrix)
        {
            return;
        }
        _proj = matrix;
        _projInv = glm::inverse(matrix);
        _transformChanged = true;
    }

    void CameraImpl::onTransformChanged()
    {
        for (auto& comp : _components)
        {
            comp.get()->onCameraTransformChanged();
        }
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

    void CameraImpl::afterLoad()
    {
        updateProjection();
    }

    bgfx::ViewId CameraImpl::renderReset(bgfx::ViewId viewId)
    {
        updateProjection();
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
        updateProjection();

        auto view = getViewMatrix();
        if (_view != view)
        {
            _view = view;
            _transformChanged = true;
        }

        if (_transformChanged)
        {
            _transformChanged = false;
            onTransformChanged();
        }
    }

    std::string CameraImpl::getViewName(const std::string& baseName) const noexcept
    {
        auto name = "Camera " + getDescName() + ": " + baseName;
        if (_scene)
        {
            name = "Scene " + _scene->getImpl().getDescName() + ": " + name;
        }
        return name;
    }

    void CameraImpl::configureView(bgfx::ViewId viewId, const std::string& name) const
    {
        bgfx::setViewName(viewId, getViewName(name).c_str());
        uint16_t clearFlags = BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;
        bgfx::setViewClear(viewId, clearFlags, 1.F, 0U);
        auto writeBuffer = _renderChain.getInput();
        if (writeBuffer)
        {
            writeBuffer->configureView(viewId);
        }
        auto vp = getCombinedViewport();
        vp.configureView(viewId);
    }

    void CameraImpl::setViewTransform(bgfx::ViewId viewId) const noexcept
    {
        auto view = getViewMatrix();
        bgfx::setViewTransform(viewId, glm::value_ptr(view), glm::value_ptr(_proj));
    }

    void CameraImpl::setEntityTransform(Entity entity, bgfx::Encoder& encoder) const noexcept
    {
        const void* transMtx = nullptr;
        if (_scene)
        {
            if (auto trans = _scene->getComponentInParent<const Transform>(entity))
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
            removeComponent(type->hash());
        }
        if (_scene)
        {
            component->init(_cam, _scene.value(), _app.value());
        }
        _components.emplace_back(std::move(component));
    }

    struct CameraComponentTypeHashFinder final
    {
        entt::id_type type;

        bool operator()(const std::shared_ptr<ICameraComponent>& comp) const noexcept
        {
            if (auto typeInfo = comp->getCameraComponentType())
            {
                return typeInfo->hash() == type;
            }
            return false;
        }
    };

    CameraImpl::Components::iterator CameraImpl::findComponent(entt::id_type type) noexcept
    {
        return std::find_if(_components.begin(), _components.end(),
            CameraComponentTypeHashFinder{ type });
    }

    CameraImpl::Components::const_iterator CameraImpl::findComponent(entt::id_type type) const noexcept
    {
        return std::find_if(_components.begin(), _components.end(),
            CameraComponentTypeHashFinder{ type });
    }

    bool CameraImpl::removeComponent(entt::id_type type) noexcept
    {
        auto itr = findComponent(type);
        auto found = itr != _components.end();
        if (found)
        {
            _components.erase(itr);
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

    ConstCameraComponentRefs CameraImpl::getComponents() const noexcept
    {
        ConstCameraComponentRefs refs;
        refs.reserve(_components.size());
        for (auto& comp : _components)
        {
            refs.emplace_back(*comp);
        }
        return refs;
    }

    CameraComponentRefs CameraImpl::getComponents() noexcept
    {
        CameraComponentRefs refs;
        refs.reserve(_components.size());
        for (auto& comp : _components)
        {
            refs.emplace_back(*comp);
        }
        return refs;
    }

    void CameraImpl::setViewport(const glm::vec4& viewport) noexcept
    {
        _viewport = viewport;
    }

    const glm::vec4& CameraImpl::getViewport() const noexcept
    {
        return _viewport;
    }

    void CameraImpl::setBaseViewport(const std::optional<Viewport>& viewport) noexcept
    {
        if(_baseViewport == viewport)
        {
            return;
        }
        _baseViewport = viewport;
        updateProjection();
    }

    const std::optional<Viewport>& CameraImpl::getBaseViewport() const noexcept
    {
        return _baseViewport;
    }

    Viewport CameraImpl::getCombinedViewport() const noexcept
    {
        Viewport vp;
        if (_baseViewport)
        {
            vp = _baseViewport.value();
        }
        else if (_scene)
        {
            vp = _scene->getCurrentViewport();
        }
        else if (_app)
        {
            vp = Viewport(_app->getWindow().getPixelSize());
        }
        return vp * _viewport;
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

    glm::mat4 CameraImpl::getViewMatrix() const noexcept
    {
        if (auto trans = getTransform())
        {
            return trans->getWorldInverse();
        }
        return glm::mat4(1);
    }

    glm::mat4 CameraImpl::getViewInverse() const noexcept
    {
        if (auto trans = getTransform())
        {
            return trans->getWorldMatrix();
        }
        return glm::mat4(1);
    }

    glm::mat4 CameraImpl::getScreenViewMatrix() const noexcept
    {
        // y = -y since screen origin is the top-left corner
        static const glm::vec3 invy(1.F, -1.F, 1.F);
        return glm::scale(glm::mat4(1.F), invy) * getViewMatrix();
    }

    Ray CameraImpl::screenPointToRay(const glm::vec3& point) const noexcept
    {
        return Ray::unproject(point, getScreenViewMatrix(), _proj, getCombinedViewport());
    }

    Ray CameraImpl::viewportPointToRay(const glm::vec3& point) const noexcept
    {
        return Ray::unproject(point, getScreenViewMatrix(), _proj);
    }

    glm::vec3 CameraImpl::worldToScreenPoint(const glm::vec3& point) const noexcept
    {
        return glm::project(point, getScreenViewMatrix(), _proj, getCombinedViewport().getValues());
    }

    glm::vec3 CameraImpl::worldToViewportPoint(const glm::vec3& point) const noexcept
    {
        return glm::project(point, getScreenViewMatrix(), _proj, Viewport().getValues());
    }

    glm::vec3 CameraImpl::screenToWorldPoint(const glm::vec3& point) const noexcept
    {
        return glm::unProject(point, getScreenViewMatrix(), _proj, getCombinedViewport().getValues());
    }

    glm::vec3 CameraImpl::viewportToWorldPoint(const glm::vec3& point) const noexcept
    {
        return glm::unProject(point, getScreenViewMatrix(), _proj, Viewport().getValues());
    }

    glm::vec3 CameraImpl::viewportToScreenPoint(const glm::vec3& point) const noexcept
    {
        auto z = point.z;
        auto p = getCombinedViewport().viewportToScreenPoint(point);
        return glm::vec3(p, z);
    }

    glm::vec3 CameraImpl::screenToViewportPoint(const glm::vec3& point) const noexcept
    {
        auto z = point.z;
        auto p = getCombinedViewport().screenToViewportPoint(point);
        return glm::vec3(p, z);
    }

    bool CameraImpl::isWorldPointVisible(const glm::vec3& point) const noexcept
    {
        auto screenPoint = worldToViewportPoint(point);
        return screenPoint.x > 0.F && screenPoint.x < 1.F && screenPoint.y > 0.F && screenPoint.y < 1.F;
    }

    Viewport CameraImpl::getRenderChainViewport() const noexcept
    {
        return getCombinedViewport();
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

    CameraComponentCerealListDelegate::CameraComponentCerealListDelegate(Camera& cam) noexcept
        : _cam(cam)
    {
    }

    entt::meta_any CameraComponentCerealListDelegate::create(const entt::meta_type& type)
    {
        return CameraReflectionUtils::getCameraComponent(_cam, type);
    }

    std::optional<entt::type_info> CameraComponentCerealListDelegate::getTypeInfo(const ICameraComponent& comp) const noexcept
    {
        return comp.getCameraComponentType();
    }

    ConstCameraComponentRefs CameraComponentCerealListDelegate::getList() const noexcept
    {
        const auto& cam = _cam;
        return cam.getComponents();
    }

    void CameraComponentCerealListDelegate::afterLoad() noexcept
    {
        for (auto& comp : _cam.getComponents())
        {
            comp.get().afterLoad();
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

    std::string Camera::getViewName(const std::string& baseName) const noexcept
    {
        return _impl->getViewName(baseName);
    }

    entt::id_type Camera::getId() const noexcept
    {
        return _impl->getId();
    }

    std::string Camera::toString() const noexcept
    {
        return _impl->toString();
    }

    const glm::mat4& Camera::getProjectionMatrix() const noexcept
    {
        return _impl->getProjectionMatrix();
    }

    const glm::mat4& Camera::getProjectionInverse() const noexcept
    {
        return _impl->getProjectionInverse();
    }

    glm::mat4 Camera::getViewProjectionMatrix() const noexcept
    {
        return _impl->getViewProjectionMatrix();
    }

    glm::mat4 Camera::getViewProjectionInverse() const noexcept
    {
        return _impl->getViewProjectionInverse();
    }

    BoundingBox Camera::getPlaneBounds(const Plane& plane) const noexcept
    {
        return _impl->getPlaneBounds(plane);
    }

    const CameraProjectionData& Camera::getProjection() const noexcept
    {
        return _impl->getProjection();
    }

    Camera& Camera::setProjection(const CameraProjectionData& data) noexcept
    {
        _impl->setProjection(data);
        return *this;
    }

    Camera& Camera::setPerspective(float fovy, float near, float far) noexcept
    {
        return setProjection(CameraPerspectiveData{ fovy, near, far });
    }

    Camera& Camera::setOrtho(const glm::vec2& center, float near, float far) noexcept
    {
        return setProjection(CameraOrthoData{ center, near, far });
    }

    Camera& Camera::setViewport(const glm::vec4& viewport) noexcept
    {
        _impl->setViewport(viewport);
        return *this;
    }

    const glm::vec4& Camera::getViewport() const noexcept
    {
        return _impl->getViewport();
    }

    Camera& Camera::setBaseViewport(const std::optional<Viewport>& viewport) noexcept
    {
        _impl->setBaseViewport(viewport);
        return *this;
    }

    const std::optional<Viewport>& Camera::getBaseViewport() const noexcept
    {
        return _impl->getBaseViewport();
    }

    Viewport Camera::getCombinedViewport() const noexcept
    {
        return _impl->getCombinedViewport();
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

    glm::mat4 Camera::getViewMatrix() const noexcept
    {
        return _impl->getViewMatrix();
    }

    glm::mat4 Camera::getViewInverse() const noexcept
    {
        return _impl->getViewInverse();
    }

    Camera& Camera::setCullingFilter(const EntityFilter& filter) noexcept
    {
        _impl->setCullingFilter(filter);
        return *this;
    }

    EntityView Camera::getEntities(const EntityFilter& filter) const
    {
        return getScene().getEntities(filter & getCullingFilter());
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

    ConstCameraComponentRefs Camera::getComponents() const noexcept
    {
        const auto& impl = *_impl;
        return impl.getComponents();
    }

    CameraComponentRefs Camera::getComponents() noexcept
    {
        return _impl->getComponents();
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

    bool Camera::isWorldPointVisible(const glm::vec3& point) const noexcept
    {
        return _impl->isWorldPointVisible(point);
    }

    RenderChain& Camera::getRenderChain() noexcept
    {
        return _impl->getRenderChain();
    }

    const RenderChain& Camera::getRenderChain() const noexcept
    {
        return _impl->getRenderChain();
    }
    
    void Camera::configureView(bgfx::ViewId viewId, const std::string& name) const
    {
        _impl->configureView(viewId, name);
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

    void Camera::bindMeta() noexcept
    {
        ReflectionSerializeUtils::metaSerialize<Camera>();
        SceneReflectionUtils::metaEntityComponent<Camera>("Camera")
            .ctor()
            .func<&Camera::afterLoad>("afterLoad"_hs)
            .func<&Camera::getName>("getName"_hs)
            .func<&Camera::setName, entt::as_void_t>("setName"_hs)
            .func<&Camera::isEnabled>("isEnabled"_hs)
            .func<&Camera::setEnabled, entt::as_void_t>("setEnabled"_hs)
            .func<&Camera::getProjectionMatrix>("getProjectionMatrix"_hs)
            ;
    }

    template<class Archive>
    void Camera::serialize(Archive& archive)
    {
        _impl->serialize(archive);
    }

    void Camera::afterLoad()
    {
        _impl->afterLoad();
    }

    DARMOK_IMPLEMENT_TEMPLATE_CEREAL_SERIALIZE(Camera::serialize)
}