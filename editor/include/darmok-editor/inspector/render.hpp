#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/render_scene.hpp>

namespace darmok::editor
{
    class RenderableInspectorEditor final : public ComponentObjectEditor<Renderable>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Renderable::Definition& renderable) noexcept override;
    };
}