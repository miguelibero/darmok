#pragma once

#include <darmok/scene.hpp>
#include <darmok/optional_ref.hpp>
#include <vector>
#include <memory>

namespace darmok
{
    class Mesh;

    struct AnimationFrame final
    {
        std::vector<std::shared_ptr<Mesh>> meshes;
        float duration;
    };

    class MeshComponent;
    class Scene;

    class FrameAnimationComponent final
    {
    public:
        FrameAnimationComponent(const std::vector<AnimationFrame>& frames, OptionalRef<MeshComponent> meshComp = nullptr) noexcept;
        void setFrames(const std::vector<AnimationFrame>& frames) noexcept;
        void setMeshComponent(MeshComponent& comp) noexcept;
        
        [[nodiscard]] const std::vector<AnimationFrame>& getFrames() const noexcept;
        [[nodiscard]] OptionalRef<const AnimationFrame> getCurrentFrame() const noexcept;

        void update(float deltaTime) noexcept;

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
        void init(Scene& scene, App& app) override;
        void update(float deltaTime) override;
    private:
        OptionalRef<Scene> _scene;
    };
}