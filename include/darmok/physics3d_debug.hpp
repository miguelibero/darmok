#pragma once

#ifdef _DEBUG

#include <memory>
#include <optional>
#include <unordered_set>

#include <darmok/export.h>
#include <darmok/render_scene.hpp>
#include <darmok/input.hpp>
#include <darmok/protobuf/physics3d.pb.h>

namespace darmok
{
    class Camera;
    class IFont;
    class Program;
    using ProgramDefines = std::unordered_set<std::string>;
}

namespace darmok::physics3d
{
    class PhysicsDebugRendererImpl;
    class PhysicsSystem;

    class DARMOK_EXPORT PhysicsDebugRenderer : public ITypeCameraComponent<PhysicsDebugRenderer>
    {
    public:
        using Definition = protobuf::PhysicsDebugRenderer;
        PhysicsDebugRenderer(const Definition& def = {}) noexcept;
        ~PhysicsDebugRenderer() noexcept;
        expected<void, std::string> init(Camera& cam, Scene& scene, App& app) noexcept override;
        expected<void, std::string> shutdown() noexcept override;
        expected<void, std::string> beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept override;

        bool isEnabled() const noexcept;
        PhysicsDebugRenderer& setEnabled(bool enabled) noexcept;

		static Definition createDefinition() noexcept;
		expected<void, std::string> load(const Definition& def, IComponentLoadContext& context) noexcept;

    private:
        std::unique_ptr<PhysicsDebugRendererImpl> _impl;
    };
}

#endif