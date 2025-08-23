#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/camera.hpp>

namespace darmok::editor
{
    class CameraInspectorEditor final : public ComponentObjectEditor<Camera::Definition>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Camera::Definition& cam) noexcept override;
    };
}