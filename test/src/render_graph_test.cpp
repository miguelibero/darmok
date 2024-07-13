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

    std::ofstream out("lala.graphviz");
    graph.writeGraphviz(out);
}

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

TEST_CASE("Iterable render graph dependencies", "[render-graph]")
{
    RenderGraphDefinition def;
    CameraRenderPass camPass;
    PostprocessRenderPass postPass;

    def.addPass(camPass);
    def.addPass(postPass);

    auto graph = def.compile();

    RenderGraphResources res;
    std::vector<TestCamera> cams{
        {"one"}, {"two"}, {"three"}
    };
    res.setCollection(cams.begin(), cams.end());

    graph.execute(res);


}