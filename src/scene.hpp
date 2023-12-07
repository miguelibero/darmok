#pragma once

#include <darmok/scene.hpp>
#include <bgfx/bgfx.h>

namespace darmok
{
    class SceneImpl final
    {
    public:
        SceneImpl(const bgfx::ProgramHandle& program);
        Registry& getRegistry();
        const Registry& getRegistry() const;
        void render(bgfx::ViewId viewId);

        static const bgfx::UniformHandle& getTexColorUniform();
        static const bgfx::UniformHandle& getModelViewProjUniform();
    private:
        entt::basic_registry<Entity> _registry;
        entt::scheduler _scheduler;
        bgfx::ProgramHandle _program;
    };
}
