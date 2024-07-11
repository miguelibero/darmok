#include <catch2/catch_test_macros.hpp>
#include <darmok/render_graph.hpp>
#include <string>
#include <glm/glm.hpp>
#include <bgfx/bgfx.h>
#include <fstream>

using namespace darmok;

// cannot create real textures in the tests
using TestTexture = std::string;

class GbufferRenderPass final : public IRenderPass
{
public:
    GbufferRenderPass()
        : _depthTexture{ "depth" }
        , _normalTexture{ "normal" }
        , _albedoTexture{ "albedo" }
    {
    }

    const std::string& getRenderPassName() const noexcept override
    {
        static const std::string name("GBuffer Pass");
        return name;
    }

    void renderPassDefine(RenderPassDefinition& def) override
    {
        def.getOutputs()
            .add("depth", _depthTexture)
            .add("normal", _normalTexture)
            .add("albedo", _albedoTexture);
    }

    void renderPassExecute(RenderGraphResources& res) override
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
        : _outTexture{ "lighting:"}
    {
    }

    const std::string& getRenderPassName() const noexcept override
    {
        static const std::string name("Deferred Lighting");
        return name;
    }

    void renderPassDefine(RenderPassDefinition& def) override
    {
        def.getInputs()
            .add("depth", _depthTexture)
            .add("normal", _normalTexture)
            .add("albedo", _albedoTexture);
        def.getOutputs()
            .add(_outTexture);
    }

    void renderPassExecute(RenderGraphResources& res) override
    {
        res.get("depth", _depthTexture);
        res.get("normal", _normalTexture);
        res.get("albedo", _albedoTexture);

        // render!
        _outTexture += _depthTexture.value() + "+" + _normalTexture.value() + "+" + _albedoTexture;

        res.setRef(_outTexture);
    }

private:
    OptionalRef<const TestTexture> _depthTexture;
    OptionalRef<const TestTexture> _normalTexture;
    TestTexture _albedoTexture;
    TestTexture _outTexture;
};

class PostprocessRenderPass final : public IRenderPass
{
public:
    PostprocessRenderPass()
        : _outTexture{ "postprocess:"}
    {
    }

    const std::string& getRenderPassName() const noexcept override
    {
        static const std::string name("Postprocess");
        return name;
    }

    void renderPassDefine(RenderPassDefinition& def) override
    {
        def.getInputs()
            .add<TestTexture>();
        def.getOutputs()
            .add<TestTexture>();
    }

    void renderPassExecute(RenderGraphResources& res) override
    {
        auto inTex = res.get<TestTexture>();
        // render!
        _outTexture += inTex.value();

        res.setRef(_outTexture);

    }
private:
    TestTexture _outTexture;
};

using namespace entt::literals;


TEST_CASE( "Compile graph dependencies", "[darmok-core]" ) {
    RenderGraph graph;

    GbufferRenderPass gbufferPass;
    DeferredLightingRenderPass lightingPass;
    PostprocessRenderPass postPass;

    graph.addPass(gbufferPass);
    graph.addPass(lightingPass);
    graph.addPass(postPass);

    graph.compile();
    auto res = graph.execute();

    auto outTexture = res.get<TestTexture>();

    REQUIRE(outTexture);
    REQUIRE(outTexture.value() == "postprocess:lighting:depth+normal+albedo");

    std::ofstream out("lala.graphviz");
    graph.writeGraphviz(out);
}