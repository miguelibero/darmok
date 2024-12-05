#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/transform.hpp>

namespace darmok::editor
{
    class TransformInspectorEditor final : public ITypeObjectEditor<Transform>
    {
    public:
        bool render(Transform& trans) noexcept override;
    };
}