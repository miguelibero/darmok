#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/transform.hpp>

namespace darmok::editor
{
    class TransformInspectorEditor final : public EntityComponentObjectEditor<Transform>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Transform::Definition& trans) noexcept override;
    };
}