
#include <darmok/anim.hpp>
#include <darmok/mesh.hpp>

namespace darmok
{
    FrameAnimationComponent::FrameAnimationComponent(const std::vector<AnimationFrame>& frames, OptionalRef<MeshComponent> meshComp) noexcept
        : _frames(frames)
        , _currentFrame(0)
        , _timeSinceLastFrame(0.F)
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

    void FrameAnimationComponent::update(float deltaTime) noexcept
    {
        _timeSinceLastFrame += deltaTime;
        if (_frames.empty())
        {
            return;
        }
        auto frame = getCurrentFrame();
        if (!frame)
        {
            return;
        }

        while (_timeSinceLastFrame > frame->duration)
        {
            _currentFrame = (_currentFrame + 1) % _frames.size();
            frame = getCurrentFrame();
            _timeSinceLastFrame -= frame->duration;
        }
        if (_meshComp)
        {
            _meshComp->setMeshes(frame->meshes);
        }
    }

    void FrameAnimationUpdater::update(float deltaTime)
    {
        auto anims = getRegistry().view<FrameAnimationComponent>();
        for (auto [entity, anim] : anims.each())
        {
            anim.update(deltaTime);
        }
    }
}