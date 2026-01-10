#pragma once

#include <darmok-editor/physics3d.hpp>
#include <darmok/physics3d.hpp>
#include <darmok/physics3d_character.hpp>
#include <darmok/physics3d_debug.hpp>
#include <darmok-editor/inspector/physics3d.hpp>

namespace darmok::editor
{

    expected<void, std::string> Physics3dEditorAppComponent::init(EditorApp& app) noexcept
    {
        _app = app;
        auto& inspector = app.getInspectorView();
        DARMOK_TRY(inspector.addEditor<Physics3dShapeInspectorEditor>());
        DARMOK_TRY(inspector.addEditor<Physics3dBodyInspectorEditor>());
        DARMOK_TRY(inspector.addEditor<Physics3dCharacterControllerInspectorEditor>());
        DARMOK_TRY(inspector.addEditor<Physics3dSystemInspectorEditor>());
        DARMOK_TRY(app.getSceneView().addGizmo<Physics3dShapeGizmo>());
        return {};
    }

    expected<void, std::string> Physics3dEditorAppComponent::renderMainMenu(MainMenuSection section) noexcept
    {
        if (!_app)
        {
            return unexpected{ "not initialized" };
        }
        if (section == MainMenuSection::AddEntityComponent)
        {
            if (ImGui::BeginMenu("Physics3d"))
            {
                DARMOK_TRY(_app->drawEntityComponentMenu<physics3d::PhysicsBody>("Body"));
                DARMOK_TRY(_app->drawEntityComponentMenu<physics3d::PhysicsBody>("Character", physics3d::PhysicsBody::createCharacterDefinition()));
                DARMOK_TRY(_app->drawEntityComponentMenu<physics3d::CharacterController>("Character Controller"));
                ImGui::EndMenu();
            }
        }
        else if (section == MainMenuSection::AddSceneComponent)
        {
            DARMOK_TRY(_app->drawSceneComponentMenu<physics3d::PhysicsSystem>("Physics3d System"));
        }
        else if (section == MainMenuSection::AddCameraComponent)
        {
            DARMOK_TRY(_app->drawCameraComponentMenu<physics3d::PhysicsDebugRenderer>("Physics3d Debug"));
        }
        return {};
    }

    expected<void, std::string> Physics3dEditorAppComponent::shutdown() noexcept
    {
        _app.reset();
        return {};
    }

    expected<void, std::string> Physics3dShapeGizmo::init(Camera& cam, Scene& scene, SceneGizmosRenderer& renderer) noexcept
    {
        _scene = scene;
        _renderer = renderer;
        return {};
    }

    expected<void, std::string> Physics3dShapeGizmo::shutdown() noexcept
    {
        _scene.reset();
        _renderer.reset();
        return {};
    }

    expected<void, std::string> Physics3dShapeGizmo::render(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        if (!_renderer)
        {
            return unexpected<std::string>{"renderer not initialized"};
        }
        auto& layout = _renderer->getVertexLayout();
        if (layout.getStride() == 0)
        {
            return unexpected<std::string>{"empty vertex layout"};
        }
        using namespace physics3d;
        auto entity = _renderer->getSelectedEntity();
        std::optional<PhysicsShape> shape;
        if (auto body = _scene->getComponent<PhysicsBody>(entity))
        {
            shape = body->getShape();
        }
        if (auto ctrl = _scene->getComponent<CharacterController>(entity))
        {
            shape = ctrl->getShape();
        }
        if (!shape)
        {
            return {};
        }
        MeshData meshData;
        static constexpr unsigned int lod = 8;
        if (auto cube = std::get_if<Cube>(&*shape))
        {
            meshData = MeshData{ *cube, Mesh::Definition::FillOutline };
        }
        else if (auto sphere = std::get_if<Sphere>(&*shape))
        {
            meshData = MeshData{ *sphere, lod };
        }
        else if (auto capsule = std::get_if<Capsule>(&*shape))
        {
            meshData = MeshData{ *capsule, lod };
        }
        else if (auto polygon = std::get_if<Polygon>(&*shape))
        {
            meshData = MeshData{ *polygon };
        }
        else if (auto bbox = std::get_if<BoundingBox>(&*shape))
        {
            meshData = MeshData{ *bbox, Mesh::Definition::FillOutline };
        }

        auto meshResult = meshData.createMesh(layout, { .type = Mesh::Definition::Transient });
        if (!meshResult)
        {
            return unexpected{ std::move(meshResult).error() };
        }
        return _renderer->renderMesh(entity, meshResult.value(), Colors::red(), Material::Definition::Line);
    }
}