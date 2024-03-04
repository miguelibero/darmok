#pragma once

#include <darmok/scene.hpp>
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include <vector>
#include <memory>

namespace darmok
{
    class SceneImpl final
    {
    public:
        SceneImpl();
        void addRenderer(std::unique_ptr<ISceneRenderer>&& renderer);
        void addLogicUpdater(std::unique_ptr<ISceneLogicUpdater>&& updater);

        EntityRegistry& getRegistry();
        const EntityRegistry& getRegistry() const;

        void init();
        void updateLogic(float dt);
        void render(bgfx::ViewId viewId);
    private:
        bool _init;
        std::vector<std::unique_ptr<ISceneRenderer>> _renderers;
        std::vector<std::unique_ptr<ISceneLogicUpdater>> _logicUpdaters;

        EntityRegistry _registry;

        void onCameraAdded(Entity entity);
    };
}
