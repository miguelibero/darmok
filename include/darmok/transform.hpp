#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <glm/detail/type_quat.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <unordered_set>

namespace darmok
{
    class DARMOK_EXPORT Transform final
    {
    public:
        static constexpr auto in_place_delete = true;

        Transform(const glm::mat4& mat, const OptionalRef<Transform>& parent = std::nullopt) noexcept;
        Transform(const glm::vec3& position = glm::vec3(), const glm::quat& rotation = glm::vec3(), const glm::vec3& scale = glm::vec3(1), const OptionalRef<Transform>& parent = std::nullopt) noexcept;
        Transform(const OptionalRef<Transform>& parent, const glm::vec3& position = glm::vec3(), const glm::quat& rotation = glm::vec3(), const glm::vec3& scale = glm::vec3(1)) noexcept;
        ~Transform() noexcept;
        std::string toString() const noexcept;

        const std::string& getName() const noexcept;
        Transform& setName(const std::string& name) noexcept;

        const glm::vec3& getPosition() const noexcept;
        const glm::quat& getRotation() const noexcept;
        const glm::vec3& getScale() const noexcept;
        Transform& setPosition(const glm::vec3& v) noexcept;
        Transform& setRotation(const glm::quat& v) noexcept;
        Transform& setScale(const glm::vec3& v) noexcept;
        Transform& setLocalMatrix(const glm::mat4& v) noexcept;

        glm::vec3 getWorldPosition() const noexcept;
        glm::vec3 getWorldScale() const noexcept;
        glm::quat getWorldRotation() const noexcept;
        glm::vec3 worldToLocalPoint(const glm::vec3& point) const noexcept;
        glm::vec3 localToWorldPoint(const glm::vec3& point) const noexcept;

        OptionalRef<const Transform> getParent() const noexcept;
        OptionalRef<Transform> getParent() noexcept;
        Transform& setParent(const OptionalRef<Transform>& parent);

        using Children = std::unordered_set<std::reference_wrapper<Transform>>;
        const Children& getChildren() const noexcept;

        const glm::mat4& getLocalMatrix() const noexcept;
        const glm::mat4& getLocalInverse() const noexcept;

        const glm::mat4& getWorldMatrix() const noexcept;
        const glm::mat4& getWorldInverse() const noexcept;

        bool update() noexcept;
        void reset() noexcept;

        glm::vec3 getEulerAngles() const noexcept;
        glm::vec3 getForward() const noexcept;
        glm::vec3 getRight() const noexcept;
        glm::vec3 getUp() const noexcept;

        Transform& setEulerAngles(const glm::vec3& v) noexcept;
        Transform& rotate(const glm::vec3& v) noexcept;
        Transform& lookDir(const glm::vec3& v, const glm::vec3& up = glm::vec3(0, 1, 0)) noexcept;
        Transform& lookAt(const glm::vec3& v, const glm::vec3& up = glm::vec3(0, 1, 0)) noexcept;
        Transform& setForward(const glm::vec3& v) noexcept;

    private:
        // should not use PIMPL here since we want consecutive memory
        glm::vec3 _position;
        glm::quat _rotation;
        glm::vec3 _scale;
        std::string _name;

        glm::mat4 _localMatrix;
        glm::mat4 _localInverse;
        glm::mat4 _worldMatrix;
        glm::mat4 _worldInverse;

        bool _matrixChanged;
        bool _parentChanged;
        OptionalRef<Transform> _parent;
        Children _children;

        void setMatrixChanged() noexcept;
        void setParentChanged() noexcept;
        void setChildrenParentChanged() noexcept;
    };
}