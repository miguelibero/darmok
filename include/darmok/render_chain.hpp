#pragma once

#include <memory>
#include <optional>
#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/texture.hpp>
#include <darmok/viewport.hpp>
#include <darmok/uniform.hpp>
#include <bgfx/bgfx.h>

namespace darmok
{
    class Texture;

    class DARMOK_EXPORT FrameBuffer
    {
    public:
        FrameBuffer(const glm::uvec2& size, bool depth = true) noexcept;
        ~FrameBuffer() noexcept;
        FrameBuffer(const FrameBuffer& other) = delete;
        FrameBuffer& operator=(const FrameBuffer& other) = delete;
        FrameBuffer(FrameBuffer&& other);
        FrameBuffer& operator=(FrameBuffer&& other);

        const std::shared_ptr<Texture>& getTexture() const noexcept;
        const std::shared_ptr<Texture>& getDepthTexture() const noexcept;
        const bgfx::FrameBufferHandle& getHandle() const noexcept;
        void configureView(bgfx::ViewId viewId) const noexcept;
        const glm::uvec2& getSize() const noexcept;
    private:
        std::shared_ptr<Texture> _colorTex;
        std::shared_ptr<Texture> _depthTex;
        bgfx::FrameBufferHandle _handle;

        static TextureConfig createColorConfig(const glm::uvec2& size) noexcept;
        static TextureConfig createDepthConfig(const glm::uvec2& size) noexcept;
    };

    class RenderChain;

    class BX_NO_VTABLE DARMOK_EXPORT IRenderChainStep
    {
    public:
        virtual ~IRenderChainStep() = default;
        virtual void init(RenderChain& chain) = 0;
        virtual void update(float deltaTime) {};
        virtual void updateRenderChain(FrameBuffer& readBuffer, OptionalRef<FrameBuffer> writeBuffer) = 0;
        virtual bgfx::ViewId renderReset(bgfx::ViewId viewId) { return viewId; };
        virtual void render(bgfx::Encoder& encoder) noexcept = 0;
        virtual void shutdown() = 0;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IRenderChainDelegate
    {
    public:
        virtual ~IRenderChainDelegate() = default;
        virtual Viewport getRenderChainViewport() const = 0;
        virtual OptionalRef<RenderChain> getRenderChainParent() const { return nullptr; }
        virtual void onRenderChainInputChanged() { }
    };

    class DARMOK_EXPORT RenderChain final
    {
    public:
        RenderChain(IRenderChainDelegate& dlg) noexcept;
        RenderChain(std::unique_ptr<IRenderChainDelegate>&& dlg) noexcept;
        void init();
        bgfx::ViewId renderReset(bgfx::ViewId viewId) noexcept;
        void update(float deltaTime);
        void render() noexcept;
        void shutdown();

        OptionalRef<FrameBuffer> getInput() noexcept;
        OptionalRef<const FrameBuffer> getInput() const noexcept;

        void configureView(bgfx::ViewId viewId, OptionalRef<const FrameBuffer> writeBuffer) const;

        RenderChain& setOutput(const std::shared_ptr<FrameBuffer>& fb) noexcept;
        std::shared_ptr<FrameBuffer> getOutput() const noexcept;

        RenderChain& addStep(std::unique_ptr<IRenderChainStep>&& step) noexcept;

        template<typename T, typename... A>
        T& addStep(A&&... args)
        {
            auto ptr = std::make_unique<T>(std::forward<A>(args)...);
            auto& ref = *ptr;
            addStep(std::move(ptr));
            return ref;
        }

        bool removeStep(const IRenderChainStep& step) noexcept;
        bool empty() const noexcept;

    private:
        IRenderChainDelegate& _delegate;
        std::unique_ptr<IRenderChainDelegate> _delegatePointer;
        bool _running;
        std::vector<std::unique_ptr<IRenderChainStep>> _steps;
        std::vector<std::unique_ptr<FrameBuffer>> _buffers;
        std::shared_ptr<FrameBuffer> _output;
        std::optional<bgfx::ViewId> _viewId;

        OptionalRef<FrameBuffer> getReadBuffer(size_t i) const noexcept;
        OptionalRef<FrameBuffer> getWriteBuffer(size_t i) const noexcept;
        FrameBuffer& addBuffer() noexcept;
        void updateStep(size_t i);
    };

    class Program;
    class IMesh;

    class DARMOK_EXPORT ScreenSpaceRenderPass final : public IRenderChainStep
    {
    public:
        ScreenSpaceRenderPass(const std::shared_ptr<Program>& prog, const std::string& name = "", int priority = 0);
        ~ScreenSpaceRenderPass() noexcept;

        void init(RenderChain& chain) noexcept override;
        void updateRenderChain(FrameBuffer& read, OptionalRef<FrameBuffer> write) noexcept override;
        void update(float deltaTime) noexcept override;
        bgfx::ViewId renderReset(bgfx::ViewId viewId) noexcept override;
        void render(bgfx::Encoder& encoder) noexcept override;
        void shutdown() noexcept override;

        ScreenSpaceRenderPass& setTexture(const std::string& name, uint8_t stage, const std::shared_ptr<Texture>& texture) noexcept;
        ScreenSpaceRenderPass& setUniform(const std::string& name, std::optional<UniformValue> value) noexcept;

    private:
        std::string _name;
        int _priority;
        OptionalRef<FrameBuffer> _readTex;
        OptionalRef<FrameBuffer> _writeTex;
        OptionalRef<RenderChain> _chain;
        std::shared_ptr<Program> _program;
        std::unique_ptr<IMesh> _mesh;
        bgfx::UniformHandle _texUniform;
        BasicUniforms _basicUniforms;
        std::optional<bgfx::ViewId> _viewId;
        UniformContainer _uniforms;
        TextureUniformContainer _textureUniforms;
    };
}