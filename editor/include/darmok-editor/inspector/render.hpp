#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/protobuf/scene.pb.h>

namespace darmok::editor
{
    class RenderableInspectorEditor final : public EntityComponentObjectEditor<Renderable>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Renderable::Definition& renderable) noexcept override;
    };

    class PrefabInspectorEditor final : public ObjectEditor<protobuf::Prefab>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(protobuf::Prefab& prefab) noexcept override;

    private:
        std::optional<EntityId> _entityId;

        RenderResult beforeRenderAny(Any& any, protobuf::Prefab& prefab) noexcept override;
        RenderResult afterRenderAny(Any& any, protobuf::Prefab& prefab, bool changed) noexcept override;
    };
}