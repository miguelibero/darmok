
#include <darmok/anim.hpp>
#include <darmok/mesh.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/scene_filter.hpp>

namespace darmok
{
    FrameAnimation::FrameAnimation(std::vector<AnimationFrame> frames, OptionalRef<Renderable> renderable) noexcept
        : _frames(std::move(frames))
        , _currentFrame(0)
        , _timeSinceLastFrame(0.F)
        , _renderable(renderable)
    {
    }

    const std::vector<AnimationFrame>& FrameAnimation::getFrames() const noexcept
    {
        return _frames;
    }

    void FrameAnimation::setFrames(const std::vector<AnimationFrame>& frames) noexcept
    {
        _frames = frames;
    }

    void FrameAnimation::setRenderable(Renderable& renderable) noexcept
    {
        _renderable = renderable;
    }

    OptionalRef<const AnimationFrame> FrameAnimation::getCurrentFrame() const noexcept
    {
        if (_frames.empty())
        {
            return nullptr;
        }
        return _frames[_currentFrame % _frames.size()];
    }

    void FrameAnimation::update(float deltaTime) noexcept
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
        if (_renderable)
        {
            _renderable->setMesh(frame->mesh);
        }
    }

    void FrameAnimationUpdater::init(Scene& scene, App& /* app */)
    {
        _scene = scene;
    }

    void FrameAnimationUpdater::update(float deltaTime)
    {
        if (!_scene)
        {
            return;
        }
        auto entities = _scene->getUpdateEntities<FrameAnimation>();
        for (auto entity : entities)
        {
            _scene->getComponent<FrameAnimation>(entity)->update(deltaTime);
        }
    }
}