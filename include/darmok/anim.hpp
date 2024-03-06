#pragma once

#include <darmok/scene.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/mesh.hpp>
#include <vector>
#include <memory>

namespace darmok
{
    struct AnimationFrame final
    {
        std::vector<std::shared_ptr<Mesh>> meshes;
        float duration;
    };

    class FrameAnimationComponent final
    {
    public:
        FrameAnimationComponent(const std::vector<AnimationFrame>& frames, OptionalRef<MeshComponent> meshComp = nullptr) noexcept;
        const std::vector<AnimationFrame>& getFrames() const noexcept;
        void setFrames(const std::vector<AnimationFrame>& frames) noexcept;
        void setMeshComponent(MeshComponent& comp) noexcept;
        void update(float deltaTime) noexcept;
        OptionalRef<const AnimationFrame> getCurrentFrame() const noexcept;

    private:
        std::vector<AnimationFrame> _frames;
        size_t _currentFrame;
        float _timeSinceLastFrame;
        OptionalRef<MeshComponent> _meshComp;
    };

    class FrameAnimationUpdater final : public ISceneLogicUpdater
    {
    public:
        FrameAnimationUpdater() = default;
        void update(float deltaTime) override;
    };
}