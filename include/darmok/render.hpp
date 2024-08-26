#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/color_fwd.hpp>
#include <darmok/viewport.hpp>
#include <darmok/render_graph.hpp>
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
    class RenderPassDefinition;
    class IRenderGraphContext;

    class DARMOK_EXPORT BX_NO_VTABLE IRenderComponent
    {
    public:
        virtual ~IRenderComponent() = default;
        virtual void init(Camera& cam, Scene& scene, App& app) {};
        virtual void update(float deltaTime) {}
        virtual void shutdown() {};
        virtual void renderPassDefine(RenderPassDefinition& def) {};

        virtual void beforeRenderView(IRenderGraphContext& context) {};
        virtual void beforeRenderEntity(Entity entity, IRenderGraphContext& context) {};
    };

    class DARMOK_EXPORT BX_NO_VTABLE IRenderer
    {
    public:
        virtual ~IRenderer() = default;
        virtual void init(Camera& cam, Scene& scene, App& app) {};
        virtual void update(float deltaTime) {};
        virtual void renderReset() {};
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
        Renderable(const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Program>& program, const Color& color) noexcept;
        Renderable(const std::shared_ptr<Material>& material) noexcept;
        std::shared_ptr<IMesh> getMesh() const noexcept;
        Renderable& setMesh(const std::shared_ptr<IMesh>& mesh) noexcept;
        std::shared_ptr<Material> getMaterial() const noexcept;
        Renderable& setMaterial(const std::shared_ptr<Material>& material) noexcept;
        bool isEnabled() const noexcept;
        Renderable& setEnabled(bool enabled) noexcept;
        bool valid() const noexcept;
        bool render(bgfx::Encoder& encoder) const;
    private:
        bool _enabled;
        std::shared_ptr<IMesh> _mesh;
        std::shared_ptr<Material> _material;
    };

    class Program;
    class IMesh;

    class DARMOK_EXPORT ScreenRenderPass final : public IRenderPass
    {
    public:
        ScreenRenderPass() noexcept;
        ~ScreenRenderPass() noexcept;
        ScreenRenderPass& setName(const std::string& name) noexcept;
        ScreenRenderPass& setPriority(int priority);
        ScreenRenderPass& setProgram(const std::shared_ptr<Program>& prog) noexcept;
        ScreenRenderPass& setViewport(std::optional<Viewport> vp) noexcept;

        void renderPassDefine(RenderPassDefinition& def) noexcept override;
        void renderPassConfigure(bgfx::ViewId viewId) noexcept override;
        void renderPassExecute(IRenderGraphContext& context) noexcept override;
    private:
        std::string _name;
        int _priority;
        std::shared_ptr<Program> _program;
        std::unique_ptr<IMesh> _mesh;
        bgfx::UniformHandle _texUniform;
        std::optional<Viewport> _viewport;
    };
}