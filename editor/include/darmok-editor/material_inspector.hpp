#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/material.hpp>

namespace darmok::editor
{
    class MaterialInspectorEditor final : public ITypeObjectEditor<Material>
    {
    public:
        bool render(Material& mat) noexcept override;
    };
}