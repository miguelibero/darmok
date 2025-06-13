#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/transform.hpp>

namespace darmok::editor
{
    class TransformInspectorEditor final : public ITypeObjectEditor<Transform::Definition>
    {
    public:
        bool renderType(Transform::Definition& trans) noexcept override;
    };
}