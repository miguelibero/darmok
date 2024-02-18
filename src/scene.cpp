#include "scene.hpp"
#include <numeric>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <bx/math.h>
#include <glm/ext/matrix_projection.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace darmok
{
    Transform::Transform(const glm::mat4& mat, const OptionalRef<Transform>& parent)
        : _position(glm::vec3())
        , _rotation(glm::vec3())
        , _scale(glm::vec3(1.f))
        , _pivot(glm::vec3())
        , _matrix(glm::mat4())
        , _changed(false)
        , _parent(parent)
    {
        setMatrix(mat);
    }

    Transform::Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale, const glm::vec3& pivot, const OptionalRef<Transform>& parent)
        : _position(position)
        , _rotation(rotation)
        , _scale(scale)
        , _pivot(pivot)
        , _matrix(glm::mat4())
        , _changed(true)
        , _parent(parent)
    {
    }

    const glm::vec3& Transform::getPosition() const
    {
        return _position;
    }

    const glm::vec3& Transform::getRotation() const
    {
        return _rotation;
    }

    const glm::vec3& Transform::getScale() const
    {
        return _scale;
    }

    const glm::vec3& Transform::getPivot() const
    {
        return _pivot;
    }

    const OptionalRef<Transform>& Transform::getParent() const
    {
        return _parent;
    }

    OptionalRef<Transform> Transform::getParent()
    {
        return _parent;
    }

    void Transform::setParent(const OptionalRef<Transform>& parent)
    {
        _parent = parent;
    }

    bool Transform::setPosition(const glm::vec3& v)
    {
        if (v == _position)
        {
            return false;
        }
        _changed = true;
        _position = v;
        return true;
    }

    bool Transform::setRotation(const glm::vec3& v)
    {
        if (v == _rotation)
        {
            return false;
        }
        _changed = true;
        _rotation = v;
        return true;
    }

    bool Transform::setScale(const glm::vec3& v)
    {
        if (v == _scale)
        {
            return false;
        }
        _changed = true;
        _scale = v;
        return true;
    }

    bool Transform::setPivot(const glm::vec3& v)
    {
        if (v == _pivot)
        {
            return false;
        }
        _changed = true;
        _pivot = v;
        return true;
    }

    bool Transform::update()
    {
        if (!_changed)
        {
            return false;
        }
        _matrix = glm::translate(_position)
            * glm::eulerAngleYXZ(glm::radians(_rotation.y), glm::radians(_rotation.x), glm::radians(_rotation.z))
            * glm::scale(_scale)
            * glm::translate(-_pivot)
            ;
        if (_parent != nullptr)
        {
            _parent->update();
            _matrix = _parent->getMatrix() * _matrix;
        }
        _changed = false;
        return true;
    }

    void Transform::setMatrix(const glm::mat4& v)
    {
        glm::quat rotation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(v, _scale, rotation, _position, skew, perspective);
        rotation = glm::conjugate(rotation);

        float yaw = 0;
        float pitch = 0;
        float roll = 0;
        glm::extractEulerAngleYXZ(glm::mat4_cast(rotation), yaw, pitch, roll);
        _rotation = { glm::degrees(pitch), glm::degrees(yaw), glm::degrees(roll) };

        _matrix = v;
        _changed = false;
    }

    const glm::mat4& Transform::getMatrix()
    {
        update();
        return _matrix;
    }

    const glm::mat4& Transform::getMatrix() const
    {
        return _matrix;
    }

    bool Transform::bgfxConfig(Entity entity, bgfx::Encoder& encoder, Registry& registry)
    {
        auto trans = registry.try_get<Transform>(entity);
        if (trans == nullptr)
        {
            return false;
        }
        auto& mtx = trans->getMatrix();
        encoder.setTransform(glm::value_ptr(mtx));
        return true;
    }

    ViewRect::ViewRect(const ViewVec& size, const ViewVec& origin)
        : _size(size), _origin(origin)
    {
    }

    void ViewRect::setSize(const ViewVec& size)
    {
        _size = size;
    }

    void ViewRect::setOrigin(const ViewVec& origin)
    {
        _origin = origin;
    }

    const ViewVec& ViewRect::getSize() const
    {
        return _size;
    }

    const ViewVec& ViewRect::getOrigin() const
    {
        return _origin;
    }

    void ViewRect::bgfxConfig(bgfx::ViewId viewId) const
    {
        bgfx::setViewRect(viewId, _origin.x, _origin.y, _size.x, _size.y);
    }

    Camera::Camera(const glm::mat4& matrix)
        : _matrix(matrix)
    {
    }

    const glm::mat4& Camera::getMatrix() const
    {
        return _matrix;
    }

    void Camera::setMatrix(const glm::mat4& matrix)
    {
        _matrix = matrix;
    }

    SceneImpl::SceneImpl()
        : _init(false)
    {
    }

    void SceneImpl::addRenderer(std::unique_ptr<ISceneRenderer>&& renderer)
    {
        if (_init)
        {
            renderer->init(_registry);
        }
        _renderers.push_back(std::move(renderer));
    }

    void SceneImpl::addLogicUpdater(std::unique_ptr<ISceneLogicUpdater>&& updater)
    {
        if (_init)
        {
            updater->init(_registry);
        }
        _logicUpdaters.push_back(std::move(updater));
    }

    Registry& SceneImpl::getRegistry()
    {
        return _registry;
    }

    const Registry& SceneImpl::getRegistry() const
    {
        return _registry;
    }

    void SceneImpl::init()
    {
        for (auto& renderer : _renderers)
        {
            renderer->init(_registry);
        }
        for (auto& updater : _logicUpdaters)
        {
            updater->init(_registry);
        }
        _init = true;
    }

    void SceneImpl::updateLogic(float dt)
    {
        for (auto& updater : _logicUpdaters)
        {
            updater->updateLogic(dt, _registry);
        }
    }

    void SceneImpl::render(bgfx::ViewId viewId)
    {
        auto cams = _registry.view<const Camera>();

        bgfx::Encoder* encoder = bgfx::begin();

        for (auto [entity, cam] : cams.each())
        {
            auto& proj = cam.getMatrix();
            auto trans = _registry.try_get<Transform>(entity);
            const void* viewPtr = nullptr;
            if (trans != nullptr)
            {
                viewPtr = glm::value_ptr(trans->getMatrix());
            }
            bgfx::setViewTransform(viewId, viewPtr, glm::value_ptr(proj));

            auto viewRect = _registry.try_get<const ViewRect>(entity);
            if (viewRect != nullptr)
            {
                viewRect->bgfxConfig(viewId);
            }

            for (auto& renderer : _renderers)
            {
                renderer->render(*encoder, viewId, _registry);
            }
        }
        bgfx::end(encoder);
    }

    Scene::Scene()
    : _impl(std::make_unique<SceneImpl>())
    {
    }

    Scene::~Scene()
    {
    }

    Entity Scene::createEntity()
    {
        return _impl->getRegistry().create();
    }

    Registry& Scene::getRegistry()
    {
        return _impl->getRegistry();
    }

    const Registry& Scene::getRegistry() const
    {
        return _impl->getRegistry();
    }

    void Scene::init()
    {
        _impl->init();
    }

    void  Scene::updateLogic(float dt)
    {
        _impl->updateLogic(dt);
    }

    void Scene::render(bgfx::ViewId viewId)
    {
        _impl->render(viewId);
    }

    void Scene::addRenderer(std::unique_ptr<ISceneRenderer>&& renderer)
    {
        _impl->addRenderer(std::move(renderer));
    }

    void Scene::addLogicUpdater(std::unique_ptr<ISceneLogicUpdater>&& updater)
    {
        _impl->addLogicUpdater(std::move(updater));
    }

    Scene& SceneAppComponent::getScene()
    {
        return _scene;
    }

    const Scene& SceneAppComponent::getScene() const
    {
        return _scene;
    }

    void SceneAppComponent::init()
    {
        _scene.init();
    }

    void SceneAppComponent::render(bgfx::ViewId viewId)
    {
        _scene.render(viewId);
    }

    void SceneAppComponent::updateLogic(float dt)
    {
        _scene.updateLogic(dt);
    }
}