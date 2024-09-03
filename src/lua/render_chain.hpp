#pragma once 

#include <sol/sol.hpp>
#include <memory>
#include <string>
#include <darmok/optional_ref.hpp>
#include "viewport.hpp"
#include "glm.hpp"

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
        static ScreenSpaceRenderPass& addChainStep1(RenderChain& chain, const std::shared_ptr<Program>& prog);
        static ScreenSpaceRenderPass& addChainStep2(RenderChain& chain, const std::shared_ptr<Program>& prog, const std::string& name);
        static ScreenSpaceRenderPass& addChainStep3(RenderChain& chain, const std::shared_ptr<Program>& prog, const std::string& name, int priority);
        static ScreenSpaceRenderPass& addChainStep4(RenderChain& chain, const std::shared_ptr<Program>& prog, int priority);
    };
}