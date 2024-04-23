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
    {
        setMatrix(mat);
        setParent(parent);
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

    const glm::quat& Transform::getRotation() const noexcept
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
        if (v)
        {
            for (auto& child : _children)
            {
                if (child)
                {
                    child->setPending(true);
                }
            }
        }
    }

    Transform& Transform::setParent(const OptionalRef<Transform>& parent) noexcept
    {
        if (_parent == parent)
        {
            return *this;
        }
        if (_parent)
        {
            _parent->_children.erase(*this);
        }
        _parent = parent;
        _parent->_children.emplace(*this);
        if (_parent->_matrixUpdatePending)
        {
            setPending();
        }
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
            * glm::mat4_cast(_rotation)
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

    bool Transform::update() noexcept
    {
        auto changed = updateMatrix();
        updateInverse();
        return changed;
    }

    Transform& Transform::setRotation(const glm::quat& v) noexcept
    {
        if (v != _rotation)
        {
            _rotation = v;
            setPending();
        }
        return *this;
    }

    Transform& Transform::setMatrix(const glm::mat4& v) noexcept
    {
        if (_matrix != v)
        {
            glm::quat rotation{};
            glm::vec3 skew{};
            glm::vec4 perspective{};
            glm::decompose(v, _scale, _rotation, _position, skew, perspective);

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