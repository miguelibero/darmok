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

        Transform(const glm::mat4& mat, const OptionalRef<Transform>& parent = std::nullopt);
        Transform(const glm::vec3& position = glm::vec3(), const glm::vec3& rotation = glm::vec3(), const glm::vec3& scale = glm::vec3(1), const glm::vec3& pivot = glm::vec3(), const OptionalRef<Transform>& parent = std::nullopt);
    
        const glm::vec3& getPosition() const;
        const glm::vec3& getRotation() const;
        const glm::vec3& getScale() const;
        const glm::vec3& getPivot() const;
        const OptionalRef<Transform>& getParent() const;

        OptionalRef<Transform> getParent();

        Transform& setPosition(const glm::vec3& v);
        Transform& setRotation(const glm::vec3& v);
        Transform& setScale(const glm::vec3& v);
        Transform& setPivot(const glm::vec3& v);
        Transform& setParent(const OptionalRef<Transform>& parent);

        void update();
        void setMatrix(const glm::mat4& v);
        const glm::mat4& getMatrix();
        const glm::mat4& getMatrix() const;
        const glm::mat4& getInverse();
        const glm::mat4& getInverse() const;

        static bool bgfxConfig(Entity entity, bgfx::Encoder& encoder, EntityRegistry& registry);
    
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

        void setPending(bool v = true);
        bool updateMatrix();
        bool updateInverse();
    };
}