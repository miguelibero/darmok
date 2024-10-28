#pragma once

#include <darmok/render_scene.hpp>
#include <darmok/optional_ref.hpp>
#include <unordered_set>
#include <optional>
#include <vector>
#include <bgfx/bgfx.h>

namespace darmok
{
    class MaterialAppComponent;

    class DARMOK_EXPORT OcclusionCuller final : public ITypeCameraComponent<OcclusionCuller>
    {
    public:
        OcclusionCuller() noexcept;
        ~OcclusionCuller() noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        bgfx::ViewId renderReset(bgfx::ViewId viewId) noexcept override;
        void render() noexcept override;
        void shutdown() noexcept override;
        bool shouldEntityBeCulled(Entity entity) noexcept override;
    private:
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        std::optional<bgfx::ViewId> _viewId;
        OptionalRef<MaterialAppComponent> _materials;
        std::vector<bgfx::OcclusionQueryHandle> _usedQueries;
        std::vector<bgfx::OcclusionQueryHandle> _freeQueries;
        std::unordered_set<Entity> _culled;

        bgfx::OcclusionQueryHandle getQuery() noexcept;
    };

    class DARMOK_EXPORT FrustrumCuller final : public  ITypeCameraComponent<FrustrumCuller>
    {
    public:
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        bool shouldEntityBeCulled(Entity entity) noexcept override;
    private:
        OptionalRef<Camera> _cam;
    };
}