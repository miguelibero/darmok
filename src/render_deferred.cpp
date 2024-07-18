#include <darmok/render_deferred.hpp>
#include <darmok/camera.hpp>

namespace darmok
{
    void DeferredRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
		_cam = cam;
    }

    void DeferredRenderer::shutdown() noexcept
    {
		_cam.reset();
    }

	void DeferredRenderer::beforeRenderView(bgfx::ViewId viewId)
	{
		_cam->beforeRenderView(viewId);
		for (auto& comp : _components)
		{
			comp->beforeRenderView(viewId);
		}
	}

	void DeferredRenderer::beforeRenderEntity(Entity entity, bgfx::Encoder& encoder)
	{
		_cam->beforeRenderEntity(entity, encoder);
		for (auto& comp : _components)
		{
			comp->beforeRenderEntity(entity, encoder);
		}
	}

	void DeferredRenderer::addComponent(std::unique_ptr<IRenderComponent>&& comp) noexcept
	{
		_components.push_back(std::move(comp));
	}
}