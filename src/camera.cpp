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
#include <darmok/glm_serialize.hpp>

#include "scene.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <bx/math.h>
#include <fmt/format.h>

using namespace entt::literals;

namespace darmok
{
    Camera::Camera(const glm::mat4& projMatrix) noexcept
        : _view{ 1.F }
        , _proj{ projMatrix }
        , _projInv{ glm::inverse(projMatrix) }
        , _projData{ CameraOrthoData{} }
        , _viewport{ 0.F, 0.F, 1.F, 1.F }
        , _renderChain{ *this }
        , _enabled{ true }
        , _transformChanged{ false }
    {
    }

    Camera::Camera(Camera&& other) noexcept
        : _view{ std::move(other._view) }
		, _proj{ std::move(other._proj) }
		, _projInv{ std::move(other._projInv) }
		, _projData{ std::move(other._projData) }
		, _baseViewport{ std::move(other._baseViewport) }
		, _viewport{ std::move(other._viewport) }
		, _renderChain{ *this }
		, _enabled{ other._enabled }
		, _updateEnabled{ std::move(other._updateEnabled) }
		, _transformChanged{ other._transformChanged }
		, _cullingFilter{ std::move(other._cullingFilter) }
		, _components{ std::move(other._components) }
		, _scene{ std::move(other._scene) }
		, _app{ std::move(other._app) }
    {
    }

    entt::id_type Camera::getId() const noexcept
    {
        return reinterpret_cast<uintptr_t>(static_cast<const void*>(this));
    }

    std::string Camera::getName() const noexcept
    {
        if (auto trans = getTransform())
        {
            return trans->getName();
        }
        return fmt::format("{:X}", getId());
    }

    std::string Camera::toString() const noexcept
    {
        return "Camera(" + getName() + ", " + glm::to_string(getProjectionMatrix()) + ")";
    }

    Scene& Camera::getScene()
    {
        return _scene.value();
    }

    const Scene& Camera::getScene() const
    {
        return _scene.value();
    }

    bool Camera::isEnabled() const noexcept
    {
        if (_updateEnabled)
        {
            return _updateEnabled.value();
        }
        return _enabled;
    }

    Camera& Camera::setEnabled(bool enabled) noexcept
    {
        _updateEnabled = enabled;
        return *this;
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

    const glm::mat4& Camera::getProjectionInverse() const noexcept
    {
        return _projInv;
    }

    glm::mat4 Camera::getViewProjectionMatrix() const noexcept
    {
       return getProjectionMatrix() * getViewMatrix();
    }

    glm::mat4 Camera::getViewProjectionInverse() const noexcept
    {
        return getViewInverse() * getProjectionInverse();
    }

    BoundingBox Camera::getPlaneBounds(const Plane& plane) const noexcept
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

    const CameraProjectionData& Camera::getProjection() const noexcept
    {
        return _projData;
    }

    Camera& Camera::setProjection(const CameraProjectionData& data) noexcept
    {
        _projData = data;
        if (_app)
        {
            updateProjection();
        }
        return *this;
    }

    bool Camera::updateProjection() noexcept
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
            auto proj = Math::perspective(winPersp->fovy, aspect, winPersp->near, winPersp->far);
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

    void Camera::setProjectionMatrix(const glm::mat4& matrix) noexcept
    {
        if (_proj == matrix)
        {
            return;
        }
        _proj = matrix;
        _projInv = glm::inverse(matrix);
        _transformChanged = true;
    }

    void Camera::onTransformChanged()
    {
        for (auto& comp : _components)
        {
            comp.get()->onCameraTransformChanged();
        }
    }

    Camera& Camera::setCullingFilter(const EntityFilter& filter) noexcept
    {
        _cullingFilter = filter;
        return *this;
    }

    const EntityFilter& Camera::getCullingFilter() const noexcept
    {
        return _cullingFilter;
    }

    void Camera::init(Scene& scene, App& app)
    {
        _scene = scene;
        _app = app;

        for(auto comp : Components{_components})
        {
            comp->init(*this, scene, app);
        }
    }

    bgfx::ViewId Camera::renderReset(bgfx::ViewId viewId)
    {
        updateProjection();
        _renderChain.beforeRenderReset();
        for (auto comp : Components{_components})
        {
            viewId = comp->renderReset(viewId);
        }
        viewId = _renderChain.renderReset(viewId);
        return viewId;
    }

    void Camera::render()
    {
        if (!_enabled)
        {
            return;
        }
        for (auto comp : Components{_components})
        {
            comp->render();
        }
    }

    void Camera::shutdown()
    {
        {
            auto components = Components{_components};
            for (auto itr = components.rbegin(); itr != components.rend(); ++itr)
            {
                (*itr)->shutdown();
            }
        }
        _renderChain.shutdown();
    }

    void Camera::update(float deltaTime)
    {
        if (_updateEnabled)
        {
            _enabled = _updateEnabled.value();
            _updateEnabled.reset();
        }
        for (auto comp : Components{ _components })
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

    std::string Camera::getViewName(const std::string& baseName) const noexcept
    {
        auto name = "Camera " + getName() + ": " + baseName;
        if (_scene)
        {
            name = "Scene " + _scene->getImpl().getDescName() + ": " + name;
        }
        return name;
    }

    void Camera::configureView(bgfx::ViewId viewId, const std::string& name) const
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

    void Camera::setViewTransform(bgfx::ViewId viewId) const noexcept
    {
        auto view = getViewMatrix();
        bgfx::setViewTransform(viewId, glm::value_ptr(view), glm::value_ptr(_proj));
    }

    void Camera::setEntityTransform(Entity entity, bgfx::Encoder& encoder) const noexcept
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

    void Camera::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) const noexcept
    {
        setViewTransform(viewId);
        for (auto comp : Components{_components})
        {
            comp->beforeRenderView(viewId, encoder);
        }
    }

    bool Camera::shouldEntityBeCulled(Entity entity) const noexcept
    {
        for (auto comp : Components{_components})
        {
            if (comp->shouldEntityBeCulled(entity))
            {
                return true;
            }
        }
        return false;
    }

    void Camera::beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) const noexcept
    {
        setEntityTransform(entity, encoder);
        for (auto comp : Components{_components})
        {
            comp->beforeRenderEntity(entity, viewId, encoder);
        }
    }

    Camera& Camera::addComponent(std::unique_ptr<ICameraComponent>&& component) noexcept
    {
        if (auto type = component->getCameraComponentType())
        {
            removeComponent(type->hash());
        }
        if (_scene)
        {
            component->init(*this, _scene.value(), _app.value());
        }
        _components.emplace_back(std::move(component));
        return *this;
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

    Camera::Components::iterator Camera::findComponent(entt::id_type type) noexcept
    {
        return std::find_if(_components.begin(), _components.end(),
            CameraComponentTypeHashFinder{ type });
    }

    Camera::Components::const_iterator Camera::findComponent(entt::id_type type) const noexcept
    {
        return std::find_if(_components.begin(), _components.end(),
            CameraComponentTypeHashFinder{ type });
    }

    bool Camera::removeComponent(entt::id_type type) noexcept
    {
        auto itr = findComponent(type);
        auto found = itr != _components.end();
        if (found)
        {
            _components.erase(itr);
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
        return **itr;
    }

    OptionalRef<const ICameraComponent> Camera::getComponent(entt::id_type type) const noexcept
    {
        auto itr = findComponent(type);
        if (itr == _components.end())
        {
            return nullptr;
        }
        return **itr;
    }

    ConstCameraComponentRefs Camera::getComponents() const noexcept
    {
        ConstCameraComponentRefs refs;
        refs.reserve(_components.size());
        for (auto& comp : _components)
        {
            refs.emplace_back(*comp);
        }
        return refs;
    }

    CameraComponentRefs Camera::getComponents() noexcept
    {
        CameraComponentRefs refs;
        refs.reserve(_components.size());
        for (auto& comp : _components)
        {
            refs.emplace_back(*comp);
        }
        return refs;
    }

    Camera& Camera::setViewport(const glm::vec4& viewport) noexcept
    {
        if (_viewport != viewport)
        {
            _viewport = viewport;
            updateProjection();
        }
        return *this;
    }

    const glm::vec4& Camera::getViewport() const noexcept
    {
        return _viewport;
    }

    Camera& Camera::setBaseViewport(const std::optional<Viewport>& viewport) noexcept
    {
        if(_baseViewport != viewport)
        {
            _baseViewport = viewport;
            updateProjection();
        }
        return *this;
    }

    const std::optional<Viewport>& Camera::getBaseViewport() const noexcept
    {
        return _baseViewport;
    }

    Viewport Camera::getCombinedViewport() const noexcept
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
            vp = Viewport(_app->getWindow().getSize());
        }
        return vp * _viewport;
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

    glm::mat4 Camera::getViewMatrix() const noexcept
    {
        if (auto trans = getTransform())
        {
            return trans->getWorldInverse();
        }
        return glm::mat4(1);
    }

    glm::mat4 Camera::getViewInverse() const noexcept
    {
        if (auto trans = getTransform())
        {
            return trans->getWorldMatrix();
        }
        return glm::mat4(1);
    }

    glm::mat4 Camera::getScreenViewMatrix() const noexcept
    {
        // y = -y since screen origin is the top-left corner
        static const glm::vec3 invy(1.F, -1.F, 1.F);
        return glm::scale(glm::mat4(1.F), invy) * getViewMatrix();
    }

    Ray Camera::screenPointToRay(const glm::vec3& point) const noexcept
    {
        return Ray::unproject(point, getScreenViewMatrix(), _proj, getCombinedViewport());
    }

    Ray Camera::viewportPointToRay(const glm::vec3& point) const noexcept
    {
        return Ray::unproject(point, getScreenViewMatrix(), _proj);
    }

    glm::vec3 Camera::worldToScreenPoint(const glm::vec3& point) const noexcept
    {
        return glm::project(point, getScreenViewMatrix(), _proj, getCombinedViewport().getValues());
    }

    glm::vec3 Camera::worldToViewportPoint(const glm::vec3& point) const noexcept
    {
        return glm::project(point, getScreenViewMatrix(), _proj, Viewport().getValues());
    }

    glm::vec3 Camera::screenToWorldPoint(const glm::vec3& point) const noexcept
    {
        return glm::unProject(point, getScreenViewMatrix(), _proj, getCombinedViewport().getValues());
    }

    glm::vec3 Camera::viewportToWorldPoint(const glm::vec3& point) const noexcept
    {
        return glm::unProject(point, getScreenViewMatrix(), _proj, Viewport().getValues());
    }

    glm::vec3 Camera::viewportToScreenPoint(const glm::vec3& point) const noexcept
    {
        auto z = point.z;
        auto p = getCombinedViewport().viewportToScreenPoint(point);
        return glm::vec3(p, z);
    }

    glm::vec3 Camera::screenToViewportPoint(const glm::vec3& point) const noexcept
    {
        auto z = point.z;
        auto p = getCombinedViewport().screenToViewportPoint(point);
        return glm::vec3(p, z);
    }

    bool Camera::isWorldPointVisible(const glm::vec3& point) const noexcept
    {
        auto screenPoint = worldToViewportPoint(point);
        return screenPoint.x > 0.F && screenPoint.x < 1.F && screenPoint.y > 0.F && screenPoint.y < 1.F;
    }

    Viewport Camera::getRenderChainViewport() const noexcept
    {
        return getCombinedViewport();
    }

    OptionalRef<RenderChain> Camera::getRenderChainParent() const noexcept
    {
        if (_scene)
        {
            return _scene->getRenderChain();
        }
        return nullptr;
    }

    void Camera::onRenderChainChanged() noexcept
    {
        if (_app)
        {
            _app->requestRenderReset();
        }
    }

    expected<void, std::string> Camera::load(const Definition& def, IComponentLoadContext& ctxt)
    {
        if (def.has_perspective_fovy())
        {
			setPerspective(def.perspective_fovy(), def.near(), def.far());
        }
        else
        {
			setOrtho(protobuf::convert(def.ortho_center()), def.near(), def.far());
        }
        return {};
    }

    Camera& Camera::setPerspective(float fovy, float near, float far) noexcept
    {
        return setProjection(CameraPerspectiveData{ fovy, near, far });
    }

    Camera& Camera::setOrtho(const glm::vec2& center, float near, float far) noexcept
    {
        return setProjection(CameraOrthoData{ center, near, far });
    }

    EntityView Camera::getEntities(const EntityFilter& filter) const
    {
        return getScene().getEntities(filter & getCullingFilter());
    }    
}