#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/texture.hpp>
#include <darmok/viewport.hpp>
#include <darmok/uniform.hpp>

#include <memory>
#include <optional>

#include <bgfx/bgfx.h>

namespace darmok
{
    class Texture;

    class DARMOK_EXPORT FrameBuffer final
    {
    public:
        ~FrameBuffer() noexcept;
        FrameBuffer(bgfx::FrameBufferHandle handle, std::shared_ptr<Texture> colorTex, std::shared_ptr<Texture> depthTex = nullptr) noexcept;
        FrameBuffer(const FrameBuffer& other) = delete;
        FrameBuffer& operator=(const FrameBuffer& other) = delete;
        FrameBuffer(FrameBuffer&& other) noexcept;
        FrameBuffer& operator=(FrameBuffer&& other) noexcept;

        static expected<FrameBuffer, std::string> load(const glm::uvec2& size, bool depth = true) noexcept;

        const std::shared_ptr<Texture>& getTexture() const noexcept;
        const std::shared_ptr<Texture>& getDepthTexture() const noexcept;
        const bgfx::FrameBufferHandle& getHandle() const noexcept;
        void configureView(bgfx::ViewId viewId) const noexcept;
        glm::uvec2 getSize() const noexcept;
    private:
        bgfx::FrameBufferHandle _handle;
        std::shared_ptr<Texture> _colorTex;
        std::shared_ptr<Texture> _depthTex;

        static Texture::Config createColorConfig(const glm::uvec2& size) noexcept;
        static Texture::Config createDepthConfig(const glm::uvec2& size) noexcept;
    };

    class RenderChain;

    class BX_NO_VTABLE DARMOK_EXPORT IRenderChainStep
    {
    public:
        virtual ~IRenderChainStep() = default;
        virtual expected<void, std::string> init(RenderChain& chain) noexcept = 0;
        virtual expected<void, std::string> update(float deltaTime) noexcept {};
        virtual expected<void, std::string> updateRenderChain(FrameBuffer& readBuffer, OptionalRef<FrameBuffer> writeBuffer) noexcept = 0;
        virtual expected<bgfx::ViewId, std::string> renderReset(bgfx::ViewId viewId) noexcept { return viewId; };
        virtual expected<void, std::string> render(bgfx::Encoder& encoder) noexcept = 0;
        virtual expected<void, std::string> shutdown() noexcept = 0;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IRenderChainDelegate
    {
    public:
        virtual ~IRenderChainDelegate() = default;
        virtual Viewport getRenderChainViewport() const noexcept = 0;
        virtual OptionalRef<RenderChain> getRenderChainParent() const noexcept { return nullptr; }
        virtual std::string getRenderChainViewName(const std::string& baseName) const noexcept { return baseName; }
        virtual void onRenderChainChanged() noexcept { }
    };

    class DARMOK_EXPORT RenderChain final
    {
    public:
        RenderChain(IRenderChainDelegate& dlg) noexcept;
        RenderChain(std::unique_ptr<IRenderChainDelegate>&& dlg) noexcept;

		RenderChain(const RenderChain& other) = delete;
		RenderChain& operator=(const RenderChain& other) = delete;
        RenderChain(RenderChain&& other) = delete;
		RenderChain& operator=(RenderChain&& other) = delete;

        expected<void, std::string> init() noexcept;
        expected<void, std::string> beforeRenderReset() noexcept;
        expected<bgfx::ViewId, std::string> renderReset(bgfx::ViewId viewId) noexcept;
        expected<void, std::string> update(float deltaTime) noexcept;
        expected<void, std::string> render() noexcept;
        expected<void, std::string> shutdown() noexcept;

        std::string getViewName(const std::string& baseName) const noexcept;
        OptionalRef<FrameBuffer> getInput() noexcept;
        OptionalRef<const FrameBuffer> getInput() const noexcept;

        void configureView(bgfx::ViewId viewId, const std::string& name, OptionalRef<const FrameBuffer> writeBuffer = nullptr) const noexcept;

        expected<void, std::string> setOutput(const std::shared_ptr<FrameBuffer>& fb) noexcept;
        std::shared_ptr<FrameBuffer> getOutput() const noexcept;

        expected<void, std::string> addStep(std::unique_ptr<IRenderChainStep>&& step) noexcept;

        template<typename T, typename... A>
        expected<std::reference_wrapper<T>, std::string> addStep(A&&... args) noexcept
        {
            auto ptr = std::make_unique<T>(std::forward<A>(args)...);
            auto& ref = *ptr;
            auto result = addStep(std::move(ptr));
            if (!result)
            {
				return unexpected{ std::move(result).error() };
            }
            return ref;
        }

        template<typename T, typename... A>
        OptionalRef<T> tryAddStep(A&&... args) noexcept
        {
            auto result = addStep<T>(std::forward<A>(args)...);
            return result ? &result.value().get() : nullptr;
        }

        expected<bool, std::string> removeStep(const IRenderChainStep& step) noexcept;
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
        expected<std::reference_wrapper<FrameBuffer>, std::string> addBuffer() noexcept;
        expected<void, std::string> updateStep(size_t i) noexcept;
    };

    class Program;
    class Mesh;

    class DARMOK_EXPORT ScreenSpaceRenderPass final : public IRenderChainStep
    {
    public:
        ScreenSpaceRenderPass(const std::shared_ptr<Program>& prog, const std::string& name = "", int priority = 0) noexcept;
        ~ScreenSpaceRenderPass() noexcept;

        expected<void, std::string> init(RenderChain& chain) noexcept override;
        expected<void, std::string> updateRenderChain(FrameBuffer& read, OptionalRef<FrameBuffer> write) noexcept override;
        expected<void, std::string> update(float deltaTime) noexcept override;
        expected<bgfx::ViewId, std::string> renderReset(bgfx::ViewId viewId) noexcept override;
        expected<void, std::string> render(bgfx::Encoder& encoder) noexcept override;
        expected<void, std::string> shutdown() noexcept override;

        ScreenSpaceRenderPass& setTexture(const std::string& name, uint8_t stage, const std::shared_ptr<Texture>& texture) noexcept;
        ScreenSpaceRenderPass& setUniform(const std::string& name, std::optional<UniformValue> value) noexcept;

    private:
        std::string _name;
        int _priority;
        OptionalRef<FrameBuffer> _readTex;
        OptionalRef<FrameBuffer> _writeTex;
        OptionalRef<RenderChain> _chain;
        std::shared_ptr<Program> _program;
        std::unique_ptr<Mesh> _mesh;
        UniformHandle _texUniform;
        BasicUniforms _basicUniforms;
        std::optional<bgfx::ViewId> _viewId;

        UniformValueMap _uniformValues;
        UniformTextureMap _uniformTextures;
        UniformHandleContainer _uniformHandles;
    };
}