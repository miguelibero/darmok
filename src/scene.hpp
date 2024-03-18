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

        void init(Scene& sceme, App& app);
        void updateLogic(float dt);
        bgfx::ViewId render(bgfx::ViewId viewId);
        void shutdown();
    private:
        std::vector<std::unique_ptr<ISceneRenderer>> _renderers;
        std::vector<std::unique_ptr<ISceneLogicUpdater>> _logicUpdaters;
        EntityRegistry _registry;
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;
    };
}
