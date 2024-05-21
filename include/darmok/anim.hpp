#pragma once

#include <darmok/scene.hpp>
#include <darmok/optional_ref.hpp>
#include <vector>
#include <memory>

namespace darmok
{
    class IMesh;

    struct AnimationFrame final
    {
        std::shared_ptr<IMesh> mesh;
        float duration;
    };

    class Renderable;

    class FrameAnimation final
    {
    public:
        DLLEXPORT FrameAnimation(const std::vector<AnimationFrame>& frames, OptionalRef<Renderable> renderable = nullptr) noexcept;
        DLLEXPORT void setFrames(const std::vector<AnimationFrame>& frames) noexcept;
        DLLEXPORT void setRenderable(Renderable& renderable) noexcept;
        
        [[nodiscard]] DLLEXPORT const std::vector<AnimationFrame>& getFrames() const noexcept;
        [[nodiscard]] DLLEXPORT OptionalRef<const AnimationFrame> getCurrentFrame() const noexcept;

        void update(float deltaTime) noexcept;

    private:
        std::vector<AnimationFrame> _frames;
        size_t _currentFrame;
        float _timeSinceLastFrame;
        OptionalRef<Renderable> _renderable;
    };

    class FrameAnimationUpdater final : public ISceneLogicUpdater
    {
    public:
        FrameAnimationUpdater() = default;
        DLLEXPORT void init(Scene& scene, App& app) override;
        DLLEXPORT void update(float deltaTime) override;
    private:
        OptionalRef<Scene> _scene;
    };
}