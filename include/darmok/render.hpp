#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <memory>
#include <vector>
#include <stdexcept>
#include <bgfx/bgfx.h>
#include <bx/bx.h>

namespace darmok
{
    class Camera;
    class App;
    class Scene;

    class DARMOK_EXPORT BX_NO_VTABLE IRenderComponent
    {
    public:
        virtual ~IRenderComponent() = default;
        virtual void init(Camera& cam, Scene& scene, App& app) {};
        virtual void update(float deltaTime) {}
        virtual void shutdown() {};

        virtual void beforeRenderView(bgfx::ViewId viewId) {};
        virtual void beforeRenderEntity(Entity entity, bgfx::Encoder& encoder) {};
    };

    class DARMOK_EXPORT BX_NO_VTABLE IRenderer
    {
    public:
        virtual ~IRenderer() = default;
        virtual void init(Camera& cam, Scene& scene, App& app) {};
        virtual void update(float deltaTime) {};
        virtual void shutdown() {};
    };

    class Material;
    class Texture;
    class IMesh;
    class Program;

    class DARMOK_EXPORT Renderable final
    {
    public:
        Renderable(const std::shared_ptr<IMesh>& mesh = nullptr, const std::shared_ptr<Material>& material = nullptr) noexcept;
        Renderable(const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Program>& program, const std::shared_ptr<Texture>& texture) noexcept;
        Renderable(const std::shared_ptr<Material>& material) noexcept;
        std::shared_ptr<IMesh> getMesh() const noexcept;
        Renderable& setMesh(const std::shared_ptr<IMesh>& mesh) noexcept;
        std::shared_ptr<Material> getMaterial() const noexcept;
        Renderable& setMaterial(const std::shared_ptr<Material>& material) noexcept;
        bool isEnabled() const noexcept;
        Renderable& setEnabled(bool enabled) noexcept;
        bool valid() const noexcept;
        void render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const;
    private:
        bool _enabled;
        std::shared_ptr<IMesh> _mesh;
        std::shared_ptr<Material> _material;
    };
}