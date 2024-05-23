#pragma once

#include <glm/glm.hpp>
#include <glm/detail/type_quat.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <unordered_set>

namespace darmok
{
    class Transform final
    {
    public:
        static constexpr auto in_place_delete = true;

        DLLEXPORT Transform(const glm::mat4& mat, const OptionalRef<Transform>& parent = std::nullopt) noexcept;
        DLLEXPORT Transform(const glm::vec3& position = glm::vec3(), const glm::quat& rotation = glm::vec3(), const glm::vec3& scale = glm::vec3(1), const glm::vec3& pivot = glm::vec3(), const OptionalRef<Transform>& parent = std::nullopt) noexcept;
        DLLEXPORT Transform(const OptionalRef<Transform>& parent, const glm::vec3& position = glm::vec3(), const glm::quat& rotation = glm::vec3(), const glm::vec3& scale = glm::vec3(1), const glm::vec3& pivot = glm::vec3()) noexcept;

        DLLEXPORT std::string to_string() const noexcept;

        DLLEXPORT const glm::vec3& getPosition() const noexcept;
        DLLEXPORT const glm::quat& getRotation() const noexcept;
        DLLEXPORT const glm::vec3& getScale() const noexcept;
        DLLEXPORT const glm::vec3& getPivot() const noexcept;
        DLLEXPORT Transform& setPosition(const glm::vec3& v) noexcept;
        DLLEXPORT Transform& setRotation(const glm::quat& v) noexcept;
        DLLEXPORT Transform& setScale(const glm::vec3& v) noexcept;
        DLLEXPORT Transform& setPivot(const glm::vec3& v) noexcept;
        DLLEXPORT Transform& setLocalMatrix(const glm::mat4& v) noexcept;

        DLLEXPORT glm::vec3 getWorldPosition() const noexcept;
        DLLEXPORT glm::vec3 worldToLocalPoint(const glm::vec3& point) const noexcept;
        DLLEXPORT glm::vec3 localToWorldPoint(const glm::vec3& point) const noexcept;

        DLLEXPORT OptionalRef<const Transform> getParent() const noexcept;
        DLLEXPORT OptionalRef<Transform> getParent() noexcept;
        DLLEXPORT Transform& setParent(const OptionalRef<Transform>& parent) noexcept;
        DLLEXPORT const std::unordered_set<OptionalRef<Transform>>& getChildren() const noexcept;

        DLLEXPORT const glm::mat4& getLocalMatrix() const noexcept;
        DLLEXPORT const glm::mat4& getLocalInverse() const noexcept;

        DLLEXPORT const glm::mat4& getWorldMatrix() const noexcept;
        DLLEXPORT const glm::mat4& getWorldInverse() const noexcept;

        DLLEXPORT bool update() noexcept;

        DLLEXPORT glm::vec3 getEulerAngles() const noexcept;
        DLLEXPORT glm::vec3 getForward() const noexcept;
        DLLEXPORT glm::vec3 getRight() const noexcept;
        DLLEXPORT glm::vec3 getUp() const noexcept;

        DLLEXPORT Transform& setEulerAngles(const glm::vec3& v) noexcept;
        DLLEXPORT Transform& rotate(const glm::vec3& v) noexcept;
        DLLEXPORT Transform& lookDir(const glm::vec3& v, const glm::vec3& up = glm::vec3(0, 1, 0)) noexcept;
        DLLEXPORT Transform& lookAt(const glm::vec3& v, const glm::vec3& up = glm::vec3(0, 1, 0)) noexcept;
        DLLEXPORT Transform& setForward(const glm::vec3& v) noexcept;

    private:
        glm::vec3 _position;
        glm::quat _rotation;
        glm::vec3 _scale;
        glm::vec3 _pivot;

        glm::mat4 _localMatrix;
        glm::mat4 _localInverse;
        glm::mat4 _worldMatrix;
        glm::mat4 _worldInverse;

        bool _matrixChanged;
        bool _parentChanged;
        OptionalRef<Transform> _parent;
        std::unordered_set<OptionalRef<Transform>> _children;

        void setMatrixChanged() noexcept;
        void setParentChanged() noexcept;
        void setChildrenParentChanged() noexcept;
    };
}