#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/protobuf/scene.pb.h>
#include <darmok/prefab.hpp>

namespace darmok::editor
{
    class RenderableInspectorEditor final : public EntityComponentObjectEditor<Renderable>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Renderable::Definition& renderable) noexcept override;
    };

    class PrefabInspectorEditor final : public EntityComponentObjectEditor<Prefab>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Prefab::Definition& prefab) noexcept override;
    };
}