#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/transform.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok::editor
{
    class TransformInspectorEditor final : public ComponentObjectEditor<Transform>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Transform::Definition& trans) noexcept override;
    };
}