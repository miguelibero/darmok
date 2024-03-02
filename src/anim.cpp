
#include <darmok/anim.hpp>
#include <darmok/mesh.hpp>

namespace darmok
{
    FrameAnimationComponent::FrameAnimationComponent(const std::vector<AnimationFrame>& frames, OptionalRef<MeshComponent> meshComp) noexcept
        : _frames(frames)
        , _currentFrame(0)
        , _timeSinceLastFrame(0.f)
        , _meshComp(meshComp)
    {
    }

    const std::vector<AnimationFrame>& FrameAnimationComponent::getFrames() const noexcept
    {
        return _frames;
    }

    void FrameAnimationComponent::setFrames(const std::vector<AnimationFrame>& frames) noexcept
    {
        _frames = frames;
    }

    void FrameAnimationComponent::setMeshComponent(MeshComponent& comp) noexcept
    {
        _meshComp = comp;
    }

    OptionalRef<const AnimationFrame> FrameAnimationComponent::getCurrentFrame() const noexcept
    {
        if (_frames.empty())
        {
            return nullptr;
        }
        return _frames[_currentFrame % _frames.size()];
    }

    void FrameAnimationComponent::update(float dt) noexcept
    {
        _timeSinceLastFrame += dt;
        if (_frames.empty())
        {
            return;
        }
        auto frame = getCurrentFrame();

        while (_timeSinceLastFrame > frame.value().duration)
        {
            _currentFrame = (_currentFrame + 1) % _frames.size();
            frame = getCurrentFrame();
            _timeSinceLastFrame -= frame.value().duration;
        }
        if (_meshComp)
        {
            _meshComp.value().setMeshes(frame.value().meshes);
        }
    }

    void FrameAnimationUpdater::updateLogic(float dt, Registry& registry)
    {
        auto anims = registry.view<FrameAnimationComponent>();
        for (auto [entity, anim] : anims.each())
        {
            anim.update(dt);
        }
    }
}