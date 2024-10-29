#pragma once

#include <darmok/render_scene.hpp>
#include <darmok/optional_ref.hpp>
#include <unordered_set>
#include <unordered_map>
#include <optional>
#include <vector>
#include <memory>
#include <bgfx/bgfx.h>

namespace darmok
{
    class Program;

    class DARMOK_EXPORT OcclusionCuller final : public ITypeCameraComponent<OcclusionCuller>
    {
    public:
        OcclusionCuller() noexcept;
        ~OcclusionCuller() noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        bgfx::ViewId renderReset(bgfx::ViewId viewId) noexcept override;
        void update(float deltaTime) noexcept override;
        void shutdown() noexcept override;
        void beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept;
    private:
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        std::optional<bgfx::ViewId> _viewId;
        std::unique_ptr<Program> _prog;
        std::unordered_map<Entity, bgfx::OcclusionQueryHandle> _queries;
        std::vector<bgfx::OcclusionQueryHandle> _freeQueries;

        void updateQueries() noexcept;
        bgfx::OcclusionQueryHandle getQuery(Entity entity) noexcept;
        void onRenderableDestroyed(EntityRegistry& registry, Entity entity) noexcept;
        std::optional<bgfx::OcclusionQueryResult::Enum> getQueryResult(Entity entity) const noexcept;
    };

    class DARMOK_EXPORT FrustumCuller final : public  ITypeCameraComponent<FrustumCuller>
    {
    public:
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void update(float deltaTime) noexcept override;
        bool shouldEntityBeCulled(Entity entity) noexcept override;
    private:
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        std::unordered_set<Entity> _culled;

        void updateCulled() noexcept;
    };
}