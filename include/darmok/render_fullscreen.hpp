#pragma once

#include <memory>
#include <darmok/export.h>
#include <darmok/render.hpp>
#include <darmok/render_graph.hpp>

namespace darmok
{
    class DARMOK_EXPORT FullscreenRenderer final : public IRenderer, public IRenderPass
    {
    public:
        FullscreenRenderer() noexcept;

        FullscreenRenderer& setTexture(const std::shared_ptr<Texture>& texture) noexcept;
        const std::shared_ptr<Texture>& getTexture() const noexcept;

        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void renderReset() noexcept override;
        void shutdown() noexcept override;

        void renderPassDefine(RenderPassDefinition& def) noexcept override;
        void renderPassConfigure(bgfx::ViewId viewId) noexcept override;
        void renderPassExecute(IRenderGraphContext& context) noexcept override;
    private:
        OptionalRef<Camera> _cam;
        std::shared_ptr<Texture> _texture;
    };
}