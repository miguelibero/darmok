#pragma once 

#include "lua/lua.hpp"
#include "lua/viewport.hpp"
#include "lua/glm.hpp"

#include <darmok/optional_ref.hpp>

#include <memory>
#include <string>

namespace darmok
{
    class FrameBuffer;
    class Texture;

    class LuaFrameBuffer final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    };

    class RenderChain;

    class LuaRenderChain final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        static OptionalRef<FrameBuffer>::std_t getInput(RenderChain& chain) noexcept;
    };

    class ScreenSpaceRenderPass;
    class Program;

    class LuaScreenSpaceRenderPass final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        static std::reference_wrapper<ScreenSpaceRenderPass> addChainStep1(RenderChain& chain, const std::shared_ptr<Program>& prog);
        static std::reference_wrapper<ScreenSpaceRenderPass> addChainStep2(RenderChain& chain, const std::shared_ptr<Program>& prog, const std::string& name);
        static std::reference_wrapper<ScreenSpaceRenderPass> addChainStep3(RenderChain& chain, const std::shared_ptr<Program>& prog, const std::string& name, int priority);
        static std::reference_wrapper<ScreenSpaceRenderPass> addChainStep4(RenderChain& chain, const std::shared_ptr<Program>& prog, int priority);
    };
}