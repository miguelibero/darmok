#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/transform.hpp>

namespace darmok::editor
{
    class TransformInspectorEditor final : public ITypeObjectEditor<Transform>
    {
    public:
        bool renderType(Transform& trans) noexcept override;
    };
}