#include "render_chain.hpp"
#include <darmok/render_chain.hpp>
#include <darmok/render_graph.hpp>
#include "glm.hpp"

namespace darmok
{
    void LuaFrameBuffer::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<FrameBuffer>("FrameBuffer",
            sol::factories(
                [](const VarLuaVecTable<glm::uvec2>& size)
                {
                    return std::make_shared<FrameBuffer>(LuaGlm::tableGet(size));
                }
            ),
            "texture", sol::property(&FrameBuffer::getTexture),
            "size", sol::property(&FrameBuffer::getSize),
            "depth_texture", sol::property(&FrameBuffer::getDepthTexture)
        );
    }

    OptionalRef<FrameBuffer>::std_t LuaRenderChain::getInput(RenderChain& chain) noexcept
    {
        return chain.getInput();
    }

    void LuaRenderChain::bind(sol::state_view& lua) noexcept
    {
        LuaFrameBuffer::bind(lua);
        LuaScreenSpaceRenderPass::bind(lua);
        lua.new_usertype<RenderChain>("RenderChain", sol::no_constructor,
            "render_graph", sol::property(sol::resolve<RenderGraphDefinition&()>(&RenderChain::getRenderGraph)),
            "output", sol::property(&RenderChain::getOutput, &RenderChain::setOutput),
            "input", sol::property(&LuaRenderChain::getInput)
        );
    }

    ScreenSpaceRenderPass& LuaScreenSpaceRenderPass::addChainStep1(RenderChain& chain, const std::shared_ptr<Program>& prog)
    {
        return chain.addStep<ScreenSpaceRenderPass>(prog);
    }

    ScreenSpaceRenderPass& LuaScreenSpaceRenderPass::addChainStep2(RenderChain& chain, const std::shared_ptr<Program>& prog, const std::string& name)
    {
        return chain.addStep<ScreenSpaceRenderPass>(prog, name);
    }

    ScreenSpaceRenderPass& LuaScreenSpaceRenderPass::addChainStep3(RenderChain& chain, const std::shared_ptr<Program>& prog, const std::string& name, int priority)
    {
        return chain.addStep<ScreenSpaceRenderPass>(prog, name, priority);
    }

    ScreenSpaceRenderPass& LuaScreenSpaceRenderPass::addChainStep4(RenderChain& chain, const std::shared_ptr<Program>& prog, int priority)
    {
        return chain.addStep<ScreenSpaceRenderPass>(prog, "", priority);
    }

    void LuaScreenSpaceRenderPass::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<ScreenSpaceRenderPass>("ScreenSpaceRenderPass", sol::no_constructor,
            "add_chain_step", sol::overload(
                LuaScreenSpaceRenderPass::addChainStep1,
                LuaScreenSpaceRenderPass::addChainStep2,
                LuaScreenSpaceRenderPass::addChainStep3,
                LuaScreenSpaceRenderPass::addChainStep4
            )
        );
    }
}