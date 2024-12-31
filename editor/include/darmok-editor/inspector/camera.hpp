#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/camera.hpp>

namespace darmok::editor
{
    class CameraInspectorEditor final : public ITypeObjectEditor<Camera>
    {
    public:
        void init(EditorApp& app, ObjectEditorContainer& editors) noexcept override;
        bool render(Camera& cam) noexcept override;
    private:
        OptionalRef<ObjectEditorContainer> _editors;
    };
}