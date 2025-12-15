#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/physics3d.hpp>
#include <darmok/physics3d_character.hpp>

struct aiScene;

namespace darmok::editor
{
    enum class Physics3dShapeType
    {
        Cube,
        Sphere,
        Capsule,
        Polygon,
        BoundingBox,
    };

    class Physics3dShapeInspectorEditor final : public ObjectEditor<physics3d::PhysicsBody::ShapeDefinition>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Object& shape) noexcept override;
    public:
        RenderResult renderPolygon(Object& src) noexcept;
    };

    class Physics3dBodyInspectorEditor final : public ComponentObjectEditor<physics3d::PhysicsBody>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(physics3d::PhysicsBody::Definition& def) noexcept override;
    };

    class Physics3dCharacterControllerInspectorEditor final : public ComponentObjectEditor<physics3d::CharacterController>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(physics3d::CharacterController::Definition& def) noexcept override;
    };

    class Physics3dSystemInspectorEditor final : public SceneComponentObjectEditor<physics3d::PhysicsSystem>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(physics3d::PhysicsSystem::Definition& def) noexcept override;
    };
}