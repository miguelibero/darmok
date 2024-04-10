#pragma once

#include <glm/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene.hpp>

namespace darmok
{
    class Transform final
    {
    public:
        static constexpr auto in_place_delete = true;

        Transform(const glm::mat4& mat, const OptionalRef<Transform>& parent = std::nullopt) noexcept;
        Transform(const glm::vec3& position = glm::vec3(), const glm::vec3& rotation = glm::vec3(), const glm::vec3& scale = glm::vec3(1), const glm::vec3& pivot = glm::vec3(), const OptionalRef<Transform>& parent = std::nullopt) noexcept;
    
        const glm::vec3& getPosition() const noexcept;
        const glm::vec3& getRotation() const noexcept;
        const glm::vec3& getScale() const noexcept;
        const glm::vec3& getPivot() const noexcept;

        OptionalRef<const Transform> getParent() const noexcept;
        OptionalRef<Transform> getParent() noexcept;

        Transform& setPosition(const glm::vec3& v) noexcept;
        Transform& setRotation(const glm::vec3& v) noexcept;
        Transform& setScale(const glm::vec3& v) noexcept;
        Transform& setPivot(const glm::vec3& v) noexcept;
        Transform& setParent(const OptionalRef<Transform>& parent) noexcept;

        void update() noexcept;
        void setMatrix(const glm::mat4& v) noexcept;
        const glm::mat4& getMatrix() noexcept;
        const glm::mat4& getMatrix() const noexcept;
        const glm::mat4& getInverse() noexcept;
        const glm::mat4& getInverse() const noexcept;

        static bool bgfxConfig(Entity entity, bgfx::Encoder& encoder, const EntityRegistry& registry) noexcept;
    
    private:

        glm::vec3 _position;
        glm::vec3 _rotation;
        glm::vec3 _scale;
        glm::vec3 _pivot;
        glm::mat4 _matrix;
        glm::mat4 _inverse;
        bool _matrixUpdatePending;
        bool _inverseUpdatePending;
        OptionalRef<Transform> _parent;

        void setPending(bool v = true) noexcept;
        bool updateMatrix() noexcept;
        bool updateInverse() noexcept;
    };
}