#pragma once

#include <memory>
#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/render_graph.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/texture.hpp>
#include <darmok/viewport.hpp>
#include <bgfx/bgfx.h>

namespace darmok
{
    class Texture;

    class DARMOK_EXPORT RenderTexture
    {
    public:
        RenderTexture(const Viewport& vp) noexcept;
        ~RenderTexture() noexcept;
        const Texture& getTexture() const noexcept;
        const Texture& getDepthTexture() const noexcept;
        const bgfx::FrameBufferHandle& getHandle() const noexcept;
        const Viewport& getViewport() const noexcept;
        void configureView(bgfx::ViewId viewId) const noexcept;
    private:
        Texture _colorTex;
        Texture _depthTex;
        Viewport _viewport;
        bgfx::FrameBufferHandle _handle;

        static TextureConfig createColorConfig(const Viewport& vp) noexcept;
        static TextureConfig createDepthConfig(const Viewport& vp) noexcept;
    };

    class RenderGraphDefinition;

    class DARMOK_EXPORT IRenderChainStep
    {
    public:
        virtual ~IRenderChainStep() = default;
        virtual void init(RenderGraphDefinition& graph) = 0;
        virtual void updateRenderChain(RenderTexture& read, OptionalRef<RenderTexture> write) = 0;
        virtual void renderReset(){ };
        virtual void shutdown() = 0;
    };

    class DARMOK_EXPORT RenderChain final
    {
    public:
        void init(RenderGraphDefinition& graph);
        void renderReset();
        void shutdown();
        void configureView(bgfx::ViewId viewId) const noexcept;

        RenderChain& setViewport(const Viewport& vp) noexcept;
        const Viewport& getViewport() const noexcept;

        RenderChain& setRenderTexture(const std::shared_ptr<RenderTexture>& renderTex) noexcept;
        std::shared_ptr<RenderTexture> getRenderTexture() const noexcept;

        RenderChain& addStep(std::unique_ptr<IRenderChainStep>&& step) noexcept;

        template<typename T, typename... A>
        T& addStep(A&&... args)
        {
            auto ptr = std::make_unique<T>(std::forward<A>(args)...);
            auto& ref = *ptr;
            addStep(std::move(ptr));
            return ref;
        }
    private:
        OptionalRef<RenderGraphDefinition> _graph;
        std::vector<std::unique_ptr<IRenderChainStep>> _steps;
        std::vector<std::unique_ptr<RenderTexture>> _textures;
        std::shared_ptr<RenderTexture> _endTexture;
        Viewport _viewport;

        void updateTextures();
        void updateSteps();
        bool updateStep(size_t i);
        void addTexture() noexcept;
    };

    class Program;
    class IMesh;

    class DARMOK_EXPORT ScreenSpaceRenderPass final : public IRenderChainStep, IRenderPass
    {
    public:
        ScreenSpaceRenderPass() noexcept;

        ScreenSpaceRenderPass& setName(const std::string& name) noexcept;
        ScreenSpaceRenderPass& setPriority(int priority);
        ScreenSpaceRenderPass& setProgram(const std::shared_ptr<Program>& prog) noexcept;

        void init(RenderGraphDefinition& graph) noexcept override;
        void updateRenderChain(RenderTexture& read, OptionalRef<RenderTexture> write) noexcept override;
        void renderReset() noexcept override;
        void shutdown() noexcept override;

        void renderPassDefine(RenderPassDefinition& def) noexcept override;
        void renderPassConfigure(bgfx::ViewId viewId) noexcept override;
        void renderPassExecute(IRenderGraphContext& context) noexcept override;
    private:
        std::string _name;
        int _priority;
        OptionalRef<RenderTexture> _readTex;
        OptionalRef<RenderTexture> _writeTex;
        OptionalRef<RenderGraphDefinition> _graph;
        std::shared_ptr<Program> _program;
        std::unique_ptr<IMesh> _mesh;
        bgfx::UniformHandle _texUniform;
    };
}