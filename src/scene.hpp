#pragma once

#include <darmok/scene.hpp>
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <vector>

namespace darmok
{
    class SceneImpl final
    {
    public:
        static const uint64_t defaultRenderState;
        static const uint8_t defaultRenderDiscard;

        SceneImpl(const bgfx::ProgramHandle& program, uint64_t renderState = defaultRenderState, uint32_t renderDepth = 0, uint8_t renderDiscard = defaultRenderDiscard) ;
        void addRenderer(std::unique_ptr<IEntityRenderer>&& renderer);
        void addLogicUpdater(std::unique_ptr<ISceneLogicUpdater>&& updater);

        Registry& getRegistry();
        const Registry& getRegistry() const;
        void updateLogic(float dt);
        void render(bgfx::ViewId viewId);

        static const bgfx::UniformHandle& getTexColorUniform();
    private:

        std::vector<std::unique_ptr<IEntityRenderer>> _renderers;
        std::vector<std::unique_ptr<ISceneLogicUpdater>> _logicUpdaters;

        entt::basic_registry<Entity> _registry;
        entt::scheduler _scheduler;
        bgfx::ProgramHandle _program;
        uint64_t _renderState;
        uint8_t _renderDiscard;
        uint32_t _renderDepth;
    };
}
