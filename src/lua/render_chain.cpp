#include "render_chain.hpp"
#include <darmok/render_chain.hpp>
#include <darmok/render_graph.hpp>
#include "glm.hpp"

namespace darmok
{
    void LuaRenderTexture::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<RenderTexture>("RenderTexture",
            sol::factories(
                [](const VarLuaVecTable<glm::uvec2>& size)
                {
                    return RenderTexture(LuaGlm::tableGet(size));
                }
            ),
            "texture", sol::property(&RenderTexture::getTexture),
            "depth_texture", sol::property(&RenderTexture::getDepthTexture)
        );
    }

    OptionalRef<RenderTexture>::std_t LuaRenderChain::getFirstTexture(RenderChain& chain) noexcept
    {
        return chain.getFirstTexture();
    }

    void LuaRenderChain::setViewport(RenderChain& chain, VarViewport vp) noexcept
    {
        chain.setViewport(LuaViewport::tableGet(vp));
    }

    void LuaRenderChain::bind(sol::state_view& lua) noexcept
    {
        LuaRenderTexture::bind(lua);
        LuaScreenSpaceRenderPass::bind(lua);
        lua.new_usertype<RenderChain>("RenderChain", sol::no_constructor,
            "render_graph", sol::property(sol::resolve<RenderGraphDefinition&()>(&RenderChain::getRenderGraph)),
            "render_texture", sol::property(&RenderChain::getRenderTexture, &RenderChain::setRenderTexture),
            "first_texture", sol::property(&LuaRenderChain::getFirstTexture),
            "viewport", sol::property(&RenderChain::getViewport, LuaRenderChain::setViewport)
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