#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/render_scene.hpp>

namespace darmok::editor
{
    class RenderableInspectorEditor final : public ITypeObjectEditor<Renderable>
    {
    public:
        bool render(Renderable& renderable) noexcept override;
    };
}