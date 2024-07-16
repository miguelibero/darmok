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
            .add<TestTexture>("depth")
            .add<TestTexture>("normal")
            .add<TestTexture>("albedo");
    }

    void renderPassExecute(RenderGraphResources& res) override
    {
        // render!
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

    const std::string& getRenderPassName() const noexcept override
    {
        static const std::string name("Deferred Lighting");
        return name;
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

    void renderPassExecute(RenderGraphResources& res) override
    {
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
        _outTexture.content += inTex->content;

        res.setRef(_outTexture);

    }
private:
    TestTexture _outTexture;
};
/*

TEST_CASE( "Compile & run render graph dependencies", "[render-graph]" )
{
    RenderGraphDefinition def;

    GbufferRenderPass gbufferPass;
    DeferredLightingRenderPass lightingPass;
    PostprocessRenderPass postPass;

    def.addPass(gbufferPass);
    def.addPass(lightingPass);
    def.addPass(postPass);

    auto graph = def.compile();
    auto res = graph.execute();

    auto outTexture = res.get<TestTexture>();

    REQUIRE(outTexture);
    REQUIRE(outTexture->content == "postprocess:lighting:depth+normal+albedo");
}

*/

struct TestCamera final
{
    std::string name;
};

class CameraRenderPass final : public IRenderPass
{
public:
    const std::string& getRenderPassName() const noexcept override
    {
        static const std::string name("Camera");
        return name;
    }

    void renderPassDefine(RenderPassDefinition& def) override
    {
        def.getInputs()
            .add<TestCamera>();
        def.getOutputs()
            .add<TestTexture>();
    }

    void renderPassExecute(RenderGraphResources& res) override
    {
        auto cam = res.get<TestCamera>();
        res.emplaceDef<TestTexture>(cam->name);
    }
};

TEST_CASE("Subgraphs", "[render-graph]")
{
    RenderGraphDefinition def1;
    CameraRenderPass camPass1;
    def1.addPass(camPass1);

    RenderGraphDefinition def2;
    CameraRenderPass camPass2;
    PostprocessRenderPass postPass;
    def2.addPass(camPass2);
    def2.addPass(postPass);

    RenderGraphDefinition def;
    def.addChild(def1);
    def.addChild(def2);

    RenderGraphResources res;

    res.emplace<TestCamera>({ .group = def1 }, "one");
    res.emplace<TestCamera>({ .group = def2 }, "two");

    auto graph = def.compile();
    graph.execute(res);

    auto outTexture1 = res.get<TestTexture>({ .group = def1 });
    auto outTexture2 = res.get<TestTexture>({ .group = def2 });

    REQUIRE(outTexture1);
    REQUIRE(outTexture1->content == "one");

    REQUIRE(outTexture2);
    REQUIRE(outTexture2->content == "postprocess:two");

}