#include <catch2/catch_test_macros.hpp>
#include <darmok/render_graph.hpp>
#include <string>
#include <glm/glm.hpp>
#include <bgfx/bgfx.h>
#include <fstream>

using namespace darmok;

struct TestTexture final
{
    std::string content;
};

struct TestCamera final
{
    std::string name;
};

TEST_CASE("RenderGraphResources stores and retrieves data", "[render-graph]")
{
    RenderGraphResources res;

    REQUIRE(!res.get<TestTexture>());

    res.set(TestTexture{ "lala" });

    REQUIRE(res.get<TestTexture>().value().content == "lala");

    res.set(TestCamera{ "cam" });

    REQUIRE(res.get<TestCamera>().value().name == "cam");
    REQUIRE(res.size() == 2);

    res.set(TestTexture{ "lolo" }, "name");

    REQUIRE(res.get<TestTexture>("name").value().content == "lolo");

    res.remove<TestTexture>();

    REQUIRE(!res.get<TestTexture>());
    REQUIRE(res.get<TestTexture>("name").value().content == "lolo");
}

class GbufferRenderPass final : public IRenderPass
{
public:
    GbufferRenderPass()
        : _depthTexture{ "depth" }
        , _normalTexture{ "normal" }
        , _albedoTexture{ "albedo" }
    {
    }

    void renderPassDefine(RenderPassDefinition& def) override
    {
        def.getOutputs()
            .add<TestTexture>("depth")
            .add<TestTexture>("normal")
            .add<TestTexture>("albedo");
    }

    void renderPassExecute(IRenderGraphContext& context) override
    {
        // render!
        auto& res = context.getResources();
        res
            .setRef(_depthTexture, { "depth", })
            .setRef(_normalTexture, { "normal" })
            .setRef(_albedoTexture, { "albedo", });
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

    void renderPassDefine(RenderPassDefinition& def) override
    {
        def.getInputs()
            .add<TestTexture>("depth")
            .add<TestTexture>("normal")
            .add<TestTexture>("albedo");
        def.getOutputs()
            .add<TestTexture>();
    }

    void renderPassExecute(IRenderGraphContext& context) override
    {
        auto& res = context.getResources();
        _depthTexture = res.get<TestTexture>({ "depth" });
        _normalTexture = res.get<TestTexture>({ "normal" });
        res.get(_albedoTexture, { "albedo" });

        // render!
        _outTexture.content += _depthTexture->content + "+" + _normalTexture->content + "+" + _albedoTexture.content;

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

    void renderPassDefine(RenderPassDefinition& def) override
    {
        def.getInputs()
            .add<TestTexture>();
        def.getOutputs()
            .add<TestTexture>();
    }

    void renderPassExecute(IRenderGraphContext& context) override
    {
        auto& res = context.getResources();
        auto inTex = res.get<TestTexture>();
        // render!
        _outTexture.content += inTex->content;

        res.setRef(_outTexture);
    }
private:
    TestTexture _outTexture;
};

TEST_CASE( "Compile & run render graph dependencies", "[render-graph]" )
{
    RenderGraphDefinition def;

    GbufferRenderPass gbufferPass;
    DeferredLightingRenderPass lightingPass;
    PostprocessRenderPass postPass;

    def.addPass(gbufferPass);
    def.addPass(lightingPass);
    def.addPass(postPass);

    RenderGraph graph(def);

    tf::Executor executor;
    graph(executor);

    auto outTexture = graph.getResources().get<TestTexture>();

    REQUIRE(outTexture);
    REQUIRE(outTexture->content == "postprocess:lighting:depth+normal+albedo");
}

class CameraRenderPass final : public IRenderPass
{
public:
    
    void renderPassDefine(RenderPassDefinition& def) override
    {
        def.getInputs()
            .add<TestCamera>();
        def.getOutputs()
            .add<TestTexture>();
    }

    void renderPassExecute(IRenderGraphContext& context) override
    {
        auto& res = context.getResources();
        auto cam = res.get<TestCamera>();
        res.emplaceDef<TestTexture>(cam->name);
    }
};

TEST_CASE("Subgraphs", "[render-graph]")
{
    RenderGraphDefinition def1("one");
    CameraRenderPass camPass1;
    def1.addPass(camPass1);

    RenderGraphDefinition def2("two");
    CameraRenderPass camPass2;
    PostprocessRenderPass postPass;
    def2.addPass(camPass2);
    def2.addPass(postPass);

    RenderGraphDefinition def;
    def.setChild(def1);
    def.setChild(def2);

    RenderGraph graph(def);

    auto& res1 = graph.getResources(def1.id());
    auto& res2 = graph.getResources(def2.id());

    res1.emplaceDef<TestCamera>("one");
    res2.emplaceDef<TestCamera>("two");

    tf::Executor executor;
    graph(executor);

    auto outTexture1 = res1.get<TestTexture>();
    auto outTexture2 = res2.get<TestTexture>();

    REQUIRE(outTexture1);
    REQUIRE(outTexture1->content == "one");

    REQUIRE(outTexture2);
    REQUIRE(outTexture2->content == "postprocess:two");
}