#include <darmok/transform.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

/*
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
*/

namespace darmok
{
    Transform::Transform(const glm::mat4& mat, const OptionalRef<Transform>& parent)
        : _position(glm::vec3())
        , _rotation(glm::vec3())
        , _scale(glm::vec3(1))
        , _pivot(glm::vec3())
        , _matrix(glm::mat4(1))
        , _inverse(glm::mat4(1))
        , _matrixUpdatePending(false)
        , _inverseUpdatePending(false)
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
        , _matrixUpdatePending(true)
        , _inverseUpdatePending(true)
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

    void Transform::setPending(bool v)
    {
        _matrixUpdatePending = v;
        _inverseUpdatePending = v;
    }


    Transform& Transform::setParent(const OptionalRef<Transform>& parent)
    {
        _parent = parent;
        return *this;
    }

    Transform& Transform::setPosition(const glm::vec3& v)
    {
        if (v != _position)
        {
            _position = v;
            setPending();
        }
        return *this;
    }

    Transform& Transform::setRotation(const glm::vec3& v)
    {
        if (v != _rotation)
        {
            _rotation = v;
            setPending();
        }
        return *this;
    }

    Transform& Transform::setScale(const glm::vec3& v)
    {
        if (v != _scale)
        {
            _scale = v;
            setPending();
        }
        return *this;
    }

    Transform& Transform::setPivot(const glm::vec3& v)
    {
        if (v != _pivot)
        {
            _pivot = v;
            setPending();
        }
        return *this;
    }

    bool Transform::updateMatrix()
    {
        if (!_matrixUpdatePending)
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
            _parent->updateMatrix();
            _matrix = _parent->getMatrix() * _matrix;
        }
        _matrixUpdatePending = false;
        return true;
    }

    bool Transform::updateInverse()
    {
        if (!_inverseUpdatePending)
        {
            return false;
        }
        updateMatrix();
        _inverse = glm::inverse(_matrix);
        _inverseUpdatePending = false;
        return true;
    }

    void Transform::update()
    {
        updateMatrix();
        updateInverse();
    }

    void Transform::setMatrix(const glm::mat4& v)
    {
        glm::quat rotation{};
        glm::vec3 skew{};
        glm::vec4 perspective{};
        glm::decompose(v, _scale, rotation, _position, skew, perspective);

        float rx = 0;
        float ry = 0;
        float rz = 0;
        glm::extractEulerAngleXYZ(glm::mat4_cast(rotation), rx, ry, rz);
        _rotation = { glm::degrees(rx), glm::degrees(ry), glm::degrees(rz) };

        _matrix = v;
        _matrixUpdatePending = false;
        _inverseUpdatePending = true;
    }

    const glm::mat4& Transform::getMatrix()
    {
        updateMatrix();
        return _matrix;
    }

    const glm::mat4& Transform::getMatrix() const
    {
        return _matrix;
    }

    const glm::mat4& Transform::getInverse()
    {
        updateInverse();
        return _inverse;
    }

    const glm::mat4& Transform::getInverse() const
    {
        return _inverse;
    }

    bool Transform::bgfxConfig(Entity entity, bgfx::Encoder& encoder, EntityRegistry& registry)
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
}