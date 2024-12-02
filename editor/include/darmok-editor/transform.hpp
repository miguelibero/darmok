#pragma once

#include <darmok-editor/editor.hpp>

namespace darmok
{
    class Transform;
}

namespace darmok::editor
{
    class TransformInspectorEditor final : public ITypeObjectEditor<Transform>
    {
    public:
        bool render(Transform& trans) noexcept override;
    };
}