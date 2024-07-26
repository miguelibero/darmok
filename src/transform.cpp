#include <darmok/transform.hpp>
#include <darmok/math.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>

namespace darmok
{
    Transform::Transform(const glm::mat4& mat, const OptionalRef<Transform>& parent) noexcept
        : _position()
        , _rotation()
        , _scale(1)
        , _pivot()
        , _localMatrix(1)
        , _localInverse(1)
        , _worldMatrix(1)
        , _worldInverse(1)
        , _matrixChanged(false)
        , _parentChanged(false)
    {
        setParent(parent);
        setLocalMatrix(mat);
    }

    Transform::Transform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, const glm::vec3& pivot, const OptionalRef<Transform>& parent) noexcept
        : _position(position)
        , _rotation(rotation)
        , _scale(scale)
        , _pivot(pivot)
        , _localMatrix(1)
        , _localInverse(1)
        , _worldMatrix(1)
        , _worldInverse(1)
        , _matrixChanged(true)
        , _parentChanged(false)
    {
        setParent(parent);
    }

    Transform::Transform(const OptionalRef<Transform>& parent, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, const glm::vec3& pivot) noexcept
        : Transform(position, rotation, scale, pivot, parent)
    {
    }

    std::string Transform::toString() const noexcept
    {
        std::string str = glm::to_string(_localMatrix);
        if (!_name.empty())
        {
            str = _name + " " + str;
        }
        return str;
    }

    const std::string& Transform::getName() const noexcept
    {
        return _name;
    }

    Transform& Transform::setName(const std::string& name) noexcept
    {
        _name = name;
        return *this;
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
        setParentChanged();
        return *this;
    }

    const std::unordered_set<OptionalRef<Transform>>& Transform::getChildren() const noexcept
    {
        return _children;
    }

    void Transform::setMatrixChanged() noexcept
    {
        _matrixChanged = true;
        setChildrenParentChanged();
    }

    void Transform::setParentChanged() noexcept
    {
        _parentChanged = true;
        setChildrenParentChanged();
    }

    void Transform::setChildrenParentChanged() noexcept
    {
        for (auto& child : _children)
        {
            child->setParentChanged();
        }
    }

    Transform& Transform::setPosition(const glm::vec3& v) noexcept
    {
        if (v != _position)
        {
            _position = v;
            setMatrixChanged();
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

    glm::vec3 Transform::getEulerAngles() const noexcept
    {
        return glm::degrees(glm::eulerAngles(getRotation()));
    }

    glm::vec3 Transform::getForward() const noexcept
    {
        static const glm::vec3 dir(0, 0, 1);
        return getRotation() * dir;
    }

    glm::vec3 Transform::getRight() const noexcept
    {
        static const glm::vec3 dir(1, 0, 0);
        return getRotation() * dir;
    }

    glm::vec3 Transform::getUp() const noexcept
    {
        static const glm::vec3 dir(0, 1, 0);
        return getRotation() * dir;
    }

    Transform& Transform::setEulerAngles(const glm::vec3& v) noexcept
    {
        setRotation(glm::quat(glm::radians(v)));
        return *this;
    }

    Transform& Transform::rotate(const glm::vec3& v) noexcept
    {
        setRotation(getRotation() * glm::quat(glm::radians(v)));
        return *this;
    }

    Transform& Transform::setScale(const glm::vec3& v) noexcept
    {
        if (v != _scale)
        {
            _scale = v;
            setMatrixChanged();
        }
        return *this;
    }

    Transform& Transform::setPivot(const glm::vec3& v) noexcept
    {
        if (v != _pivot)
        {
            _pivot = v;
            setMatrixChanged();
        }
        return *this;
    }

    bool Transform::update() noexcept
    {
        if (_matrixChanged)
        {
            _localMatrix = Math::transform(_position, _rotation, _scale, _pivot);
            _localInverse = glm::inverse(_localMatrix);
        }
        auto changed = _parentChanged || _matrixChanged;
        if (changed)
        {
            _worldMatrix = _localMatrix;
            _worldInverse = _localInverse;
            if (_parent != nullptr)
            {
                _parent->update();
                _worldMatrix = _parent->getWorldMatrix() * _worldMatrix;
                _worldInverse = _worldInverse * _parent->getWorldInverse();
            }
            changed = true;
        }
        _matrixChanged = false;
        _parentChanged = false;
        return changed;
    }

    Transform& Transform::setRotation(const glm::quat& v) noexcept
    {
        if (v != _rotation)
        {
            _rotation = v;
            setMatrixChanged();
        }
        return *this;
    }

    Transform& Transform::setLocalMatrix(const glm::mat4& v) noexcept
    {
        if (_localMatrix != v)
        {
            glm::vec3 skew{};
            glm::vec4 perspective{};
            glm::decompose(v, _scale, _rotation, _position, skew, perspective);
            // TODO: check skew == [0, 0, 0] && persp == [0, 0, 0, 1]
            // TODO: optimize calculating inverse in next update
            _localMatrix = v;
            _localInverse = glm::inverse(_localMatrix);
            setParentChanged();
        }

        return *this;
    }

    glm::vec3 Transform::getWorldPosition() const noexcept
    {
        return _worldMatrix * glm::vec4(0, 0, 0, 1);
    }

    glm::vec3 Transform::worldToLocalPoint(const glm::vec3& point) const noexcept
    {
        return _worldMatrix * glm::vec4(point, 1);
    }

    glm::vec3 Transform::localToWorldPoint(const glm::vec3& point) const noexcept
    {
        return _worldInverse * glm::vec4(point, 1);
    }

    const glm::mat4& Transform::getLocalMatrix() const noexcept
    {
        return _localMatrix;
    }

    const glm::mat4& Transform::getLocalInverse() const noexcept
    {
        return _localInverse;
    }

    const glm::mat4& Transform::getWorldMatrix() const noexcept
    {
        return _worldMatrix;
    }

    const glm::mat4& Transform::getWorldInverse() const noexcept
    {
        return _worldInverse;
    }
}