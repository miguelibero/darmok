#pragma once

#include <darmok/export.h>
#include <darmok/render_scene.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/protobuf/camera.pb.h>

namespace darmok
{
    class Camera;

    class DARMOK_EXPORT DeferredRenderer final : public ITypeCameraComponent<DeferredRenderer>
    {
    public:
        using Definition = protobuf::DeferredRenderer;

        expected<void, std::string> init(Camera& cam, Scene& scene, App& app) noexcept override;
        expected<bgfx::ViewId, std::string> renderReset(bgfx::ViewId viewId) noexcept override;
        expected<void, std::string> shutdown() noexcept override;
    private:
        OptionalRef<Camera> _cam;
    };
}