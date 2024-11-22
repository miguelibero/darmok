#pragma once

#include <darmok/export.h>
#include <darmok/scene.hpp>
#include <darmok/optional_ref.hpp>
#include <cereal/cereal.hpp>
#include <vector>
#include <memory>

namespace darmok
{
    class IMesh;

    struct AnimationFrame final
    {
        std::shared_ptr<IMesh> mesh;
        float duration;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(mesh),
                CEREAL_NVP(duration)
            );
        }
    };

    class Renderable;

    class DARMOK_EXPORT FrameAnimation final
    {
    public:
        FrameAnimation(const std::vector<AnimationFrame>& frames, OptionalRef<Renderable> renderable = nullptr) noexcept;
        void setFrames(const std::vector<AnimationFrame>& frames) noexcept;
        void setRenderable(Renderable& renderable) noexcept;
        
        [[nodiscard]] const std::vector<AnimationFrame>& getFrames() const noexcept;
        [[nodiscard]] OptionalRef<const AnimationFrame> getCurrentFrame() const noexcept;

        void update(float deltaTime) noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP_("frames", _frames),
                CEREAL_NVP_("currentFrame", _currentFrame),
                CEREAL_NVP_("timeSinceLastFrame", _timeSinceLastFrame)
            );
        }

    private:
        std::vector<AnimationFrame> _frames;
        size_t _currentFrame;
        float _timeSinceLastFrame;
        OptionalRef<Renderable> _renderable;
    };

    class DARMOK_EXPORT FrameAnimationUpdater final : public ITypeSceneComponent<FrameAnimationUpdater>
    {
    public:
        FrameAnimationUpdater() = default;
        void init(Scene& scene, App& app) override;
        void update(float deltaTime) override;
    private:
        OptionalRef<Scene> _scene;
    };
}