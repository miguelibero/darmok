#include <darmok/transform.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace darmok
{
    Transform::Transform(const glm::mat4& mat, const OptionalRef<Transform>& parent) noexcept
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

    Transform::Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale, const glm::vec3& pivot, const OptionalRef<Transform>& parent) noexcept
        : _position(position)
        , _rotation(rotation)
        , _scale(scale)
        , _pivot(pivot)
        , _matrix()
        , _inverse()
        , _matrixUpdatePending(true)
        , _inverseUpdatePending(true)
        , _parent(parent)
    {
    }

    const glm::vec3& Transform::getPosition() const noexcept
    {
        return _position;
    }

    const glm::vec3& Transform::getRotation() const noexcept
    {
        return _rotation;
    }

    const glm::vec3& Transform::getScale() const noexcept
    {
        return _scale;
    }

    const glm::vec3& Transform::getPivot() const noexcept
    {
        return _pivot;
    }

    glm::vec3 Transform::getForward() const noexcept
    {
        auto fw = glm::vec3(0, 0, 1);
        fw = glm::rotateX(fw, glm::radians(_rotation.x));
        fw = glm::rotateY(fw, glm::radians(_rotation.y));
        fw = glm::rotateZ(fw, -glm::radians(_rotation.z));
        return fw;
    }

    OptionalRef<const Transform> Transform::getParent() const noexcept
    {
        return _parent;
    }

    OptionalRef<Transform> Transform::getParent() noexcept
    {
        return _parent;
    }

    void Transform::setPending(bool v) noexcept
    {
        _matrixUpdatePending = v;
        _inverseUpdatePending = v;
    }

    Transform& Transform::setParent(const OptionalRef<Transform>& parent) noexcept
    {
        _parent = parent;
        return *this;
    }

    Transform& Transform::setPosition(const glm::vec3& v) noexcept
    {
        if (v != _position)
        {
            _position = v;
            setPending();
        }
        return *this;
    }

    Transform& Transform::setRotation(const glm::vec3& v) noexcept
    {
        if (v != _rotation)
        {
            _rotation = v;
            setPending();
        }
        return *this;
    }

    Transform& Transform::setForward(const glm::vec3& v) noexcept
    {
        return *this;
    }

    Transform& Transform::lookDir(const glm::vec3& v, const glm::vec3& up) noexcept
    {
        return setRotation(glm::quatLookAt(glm::normalize(v), up));
    }

    Transform& Transform::lookAt(const glm::vec3& v, const glm::vec3& up) noexcept
    {
        return lookDir(v - _position);
    }

    Transform& Transform::setScale(const glm::vec3& v) noexcept
    {
        if (v != _scale)
        {
            _scale = v;
            setPending();
        }
        return *this;
    }

    Transform& Transform::setPivot(const glm::vec3& v) noexcept
    {
        if (v != _pivot)
        {
            _pivot = v;
            setPending();
        }
        return *this;
    }

    bool Transform::updateMatrix() noexcept
    {
        if (!_matrixUpdatePending)
        {
            return false;
        }
        _matrix = glm::translate(_position)
            * glm::eulerAngleXYZ(glm::radians(_rotation.x), glm::radians(_rotation.y), glm::radians(_rotation.z))
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

    bool Transform::updateInverse() noexcept
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

    void Transform::update() noexcept
    {
        updateMatrix();
        updateInverse();
    }

    Transform& Transform::setRotation(const glm::quat& v) noexcept
    {
        return setRotation(rotationQuadToVec(v));
    }

    glm::vec3 Transform::rotationQuadToVec(const glm::quat& v) noexcept
    {
        float rx = 0;
        float ry = 0;
        float rz = 0;
        glm::extractEulerAngleXYZ(glm::mat4_cast(v), rx, ry, rz);
        return { glm::degrees(rx), glm::degrees(ry), glm::degrees(rz) };
    }

    Transform& Transform::setMatrix(const glm::mat4& v) noexcept
    {
        if (_matrix != v)
        {
            glm::quat rotation{};
            glm::vec3 skew{};
            glm::vec4 perspective{};
            glm::decompose(v, _scale, rotation, _position, skew, perspective);
            _rotation = rotationQuadToVec(rotation);

            _matrix = v;
            _matrixUpdatePending = false;
            _inverseUpdatePending = true;
        }

        return *this;
    }

    const glm::mat4& Transform::getMatrix() noexcept
    {
        updateMatrix();
        return _matrix;
    }

    const glm::mat4& Transform::getMatrix() const noexcept
    {
        return _matrix;
    }

    const glm::mat4& Transform::getInverse() noexcept
    {
        updateInverse();
        return _inverse;
    }

    const glm::mat4& Transform::getInverse() const noexcept
    {
        return _inverse;
    }

    bool Transform::bgfxConfig(Entity entity, bgfx::Encoder& encoder, const EntityRegistry& registry) noexcept
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