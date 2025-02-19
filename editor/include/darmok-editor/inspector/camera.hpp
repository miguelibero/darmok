#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/camera.hpp>

#include <array>

namespace darmok::editor
{
    class CameraInspectorEditor final : public ITypeObjectEditor<Camera>
    {
    public:
        void init(EditorApp& app, ObjectEditorContainer& editors) noexcept override;
        bool renderType(Camera& cam) noexcept override;
    private:
        OptionalRef<ObjectEditorContainer> _editors;
        static const std::array<std::string, 2> _projOptions;
    };
}