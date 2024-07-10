#include <catch2/catch_test_macros.hpp>
#include <darmok/render_graph.hpp>
#include <string>
#include <glm/glm.hpp>
#include <bgfx/bgfx.h>

using namespace darmok;

// cannot create real textures in the tests
struct TestTexture final
{
    glm::uvec2 size;
    bgfx::TextureFormat::Enum format;
};

class GbufferRenderPass final : public IRenderPass
{
public:
    GbufferRenderPass()
        : _depthTexture{ glm::uvec2(1024), bgfx::TextureFormat::D16 }
        , _normalTexture{ glm::uvec2(1024), bgfx::TextureFormat::RGB8 }
        , _albedoTexture{ glm::uvec2(1024), bgfx::TextureFormat::RGBA8 }
    {
    }

    const std::string& getRenderPassName() const noexcept override
    {
        static const std::string name("GBuffer Pass");
        return name;
    }

    void onRenderPassDefine(RenderPassDefinition& def) override
    {
        def.getOutputs()
            .add("depth", _depthTexture)
            .add("normal", _normalTexture)
            .add("albedo", _albedoTexture);
    }

    void onRenderPassExecute(RenderGraphResources& res) override
    {
        // render!
        res
            .setRef("depth", _depthTexture)
            .setRef("normal", _normalTexture)
            .setRef("albedo", _albedoTexture);
    }
private:
    TestTexture _depthTexture;
    TestTexture _normalTexture;
    TestTexture _albedoTexture;
};

class DeferredLightingRenderPass final : public IRenderPass
{
public:
    DeferredLightingRenderPass()
        : _hdrTexture{ glm::uvec2(1024), bgfx::TextureFormat::RGBA8 }
    {
    }

    const std::string& getRenderPassName() const noexcept override
    {
        static const std::string name("Deferred Lighting");
        return name;
    }

    void onRenderPassDefine(RenderPassDefinition& def) override
    {
        def.getInputs()
            .add("depth", _depthTexture)
            .add("normal", _normalTexture)
            .add("albedo", _albedoTexture);
        def.getOutputs()
            .add("hdr", _hdrTexture);
    }

    void onRenderPassExecute(RenderGraphResources& res) override
    {
        res.get("depth", _depthTexture);
        res.get("normal", _depthTexture);
        res.get("albedo", _albedoTexture);
        // render!
        res.setRef("hdr", _hdrTexture);

    }

private:
    OptionalRef<const TestTexture> _depthTexture;
    OptionalRef<const TestTexture> _normalTexture;
    OptionalRef<const TestTexture> _albedoTexture;
    TestTexture _hdrTexture;
};

TEST_CASE( "Compile graph dependencies", "[darmok-core]" ) {
    RenderGraph graph;

    GbufferRenderPass gbufferPass;
    DeferredLightingRenderPass lightingPass;

    graph.addPass(gbufferPass);
    graph.addPass(lightingPass);

    graph.compile();
    graph.execute();
}