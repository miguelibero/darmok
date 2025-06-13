#include <darmok/transform.hpp>
#include <darmok/math.hpp>
#include <darmok/glm_serialize.hpp>
#include <darmok/protobuf/scene.pb.h>
#include <darmok/scene.hpp>
#include <darmok/scene_serialize.hpp>

#include <glm/ext/matrix_projection.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace entt::literals;

namespace darmok
{
    void TransformUtils::update(Definition& trans, const glm::mat4& mat) noexcept
    {
        glm::vec3 pos;
		glm::quat rot;
		glm::vec3 scale;
        Math::decompose(mat, pos, rot, scale);
		*trans.mutable_position() = protobuf::convert(pos);
        *trans.mutable_rotation() = protobuf::convert(rot);
        *trans.mutable_scale() = protobuf::convert(scale);
    }

    Transform::Transform(const glm::mat4& mat, const OptionalRef<Transform>& parent) noexcept
        : _position{}
        , _rotation{}
        , _scale{ 1.f }
        , _localMatrix{ 1.f }
        , _localInverse{ 1.f }
        , _worldMatrix{ 1.f }
        , _worldInverse{ 1.f }
        , _matrixChanged{ false }
        , _parentChanged{ false }
    {
        setParent(parent);
        setLocalMatrix(mat);
    }

    Transform::Transform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, const OptionalRef<Transform>& parent) noexcept
        : _position{ position }
        , _rotation{ rotation }
        , _scale{ scale }
        , _localMatrix{ 1.f }
        , _localInverse{ 1.f }
        , _worldMatrix{ 1.f }
        , _worldInverse{ 1.f }
        , _matrixChanged{ true }
        , _parentChanged{ false }
    {
        setParent(parent);
    }

    Transform::~Transform() noexcept
    {
        if (_parent)
        {
            _parent->_children.erase(*this);
        }
        for (auto& child : Children{ _children })
        {
            child.get().setParent(nullptr);
        }
    }

    void Transform::reset() noexcept
    {
        _position = glm::vec3{};
        _rotation = glm::quat{};
        _scale = glm::vec3{1.f};
        _localMatrix = glm::mat4{ 1.f };
        _localInverse = glm::mat4{ 1.f };
        _matrixChanged = true;
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

    OptionalRef<Transform> Transform::getParent() const noexcept
    {
        return _parent;
    }

    Transform& Transform::setParent(const OptionalRef<Transform>& parent)
    {
        if (_parent == parent)
        {
            return *this;
        }
        if (parent == this)
        {
            throw std::runtime_error{ "cannot be parent of itself" };
        }
        if (_parent)
        {
            _parent->_children.erase(*this);
        }
        _parent = parent;
        if (_parent)
        {
            _parent->_children.emplace(*this);
        }
        setParentChanged();
        return *this;
    }

    const Transform::Children& Transform::getChildren() const noexcept
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
        for (auto& child : Children{ _children })
        {
            child.get().setParentChanged();
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
        return lookDir(v - _position, up);
    }

    glm::vec3 Transform::getEulerAngles() const noexcept
    {
        return glm::eulerAngles(getRotation());
    }

    glm::vec3 Transform::getForward() const noexcept
    {
        static const glm::vec3 dir{ 0.f, 0.f, 1.f };
        return getRotation() * dir;
    }

    glm::vec3 Transform::getRight() const noexcept
    {
        static const glm::vec3 dir{ 1.f, 0.f, 0.f };
        return getRotation() * dir;
    }

    glm::vec3 Transform::getUp() const noexcept
    {
        static const glm::vec3 dir{ 0.f, 1.f, 0.f };
        return getRotation() * dir;
    }

    Transform& Transform::setEulerAngles(const glm::vec3& v) noexcept
    {
        setRotation(glm::quat{ v });
        return *this;
    }

    Transform& Transform::rotate(const glm::vec3& v) noexcept
    {
        setRotation(getRotation() * glm::quat{ v });
        return *this;
    }

    Transform& Transform::rotateAround(const glm::vec3& point, const glm::vec3& axis, float angle) noexcept
    {
        auto pos = _position - point;
        auto quat = glm::angleAxis(angle, glm::normalize(axis));
        setPosition((quat * pos) + point);
        setRotation(quat * _rotation);
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

    bool Transform::update() noexcept
    {
        if (_matrixChanged)
        {
            _localMatrix = Math::transform(_position, _rotation, _scale);
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
            Math::decompose(v, _position, _rotation, _scale);
            // TODO: optimize calculating inverse in next update
            _localMatrix = v;
            _localInverse = glm::inverse(_localMatrix);
            setParentChanged();
        }

        return *this;
    }

    glm::vec3 Transform::getWorldPosition() const noexcept
    {
        return glm::vec3{ _worldMatrix[3] };
    }

    glm::vec3 Transform::getWorldScale() const noexcept
    {
        return {
            glm::length(glm::vec3{_worldMatrix[0]}),
            glm::length(glm::vec3{_worldMatrix[1]}),
            glm::length(glm::vec3{_worldMatrix[2]})
        };
    }

    glm::quat Transform::getWorldRotation() const noexcept
    {
        return glm::quat_cast(_worldMatrix);
    }

    glm::vec3 Transform::getWorldDirection() const noexcept
    {
        static const glm::vec3 forward{ 0.f, 0.f, 1.f };
        return getWorldRotation() * forward;
    }

    glm::vec3 Transform::worldToLocalPoint(const glm::vec3& point) const noexcept
    {
        return _worldMatrix * glm::vec4{ point, 1.f };
    }

    glm::vec3 Transform::localToWorldPoint(const glm::vec3& point) const noexcept
    {
        return _worldInverse * glm::vec4{ point, 1.f };
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

    expected<void, std::string> Transform::load(const Definition& def, IComponentLoadContext& ctxt)
    {
		setName(def.name());
        setPosition(protobuf::convert(def.position()));
        setScale(protobuf::convert(def.scale()));
        setRotation(protobuf::convert(def.rotation()));
        auto parentEntity = ctxt.getEntity(def.parent());
        setParent(ctxt.getScene().getComponent<Transform>(parentEntity));
        return {};
    }
}