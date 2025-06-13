#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/camera.hpp>

namespace darmok::editor
{
    class CameraInspectorEditor final : public ITypeObjectEditor<Camera::Definition>
    {
    public:
        void init(EditorApp& app, ObjectEditorContainer& editors) noexcept override;
        bool renderType(Camera::Definition& cam) noexcept override;
    private:
        OptionalRef<ObjectEditorContainer> _editors;
    };
}