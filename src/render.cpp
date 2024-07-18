#include <darmok/render.hpp>
#include <darmok/material.hpp>
#include <darmok/mesh.hpp>
#include <darmok/program.hpp>
#include <darmok/camera.hpp>

namespace darmok
{
	void Renderer::init(Camera& cam, Scene& scene, App& app)
	{
		_cam = cam;
		_scene = scene;
		_app = app;
		for (auto& comp : _components)
		{
			comp->init(cam, scene, app);
		}
	}

	void Renderer::update(float deltaTime)
	{
		for (auto& comp : _components)
		{
			comp->update(deltaTime);
		}
	}

	void Renderer::shutdown()
	{
		_cam.reset();
		_scene.reset();
		_app.reset();
		for (auto& comp : _components)
		{
			comp->shutdown();
		}
	}

	void Renderer::addComponent(std::unique_ptr<IRenderComponent>&& comp)
	{
		if (_cam)
		{
			comp->init(_cam.value(), _scene.value(), _app.value());
		}
		_components.push_back(std::move(comp));
	}

	void Renderer::beforeRenderView(bgfx::ViewId viewId)
	{
		_cam->beforeRenderView(viewId);
		for (auto& comp : _components)
		{
			comp->beforeRenderView(viewId);
		}
	}

	void Renderer::beforeRenderEntity(Entity entity, bgfx::Encoder& encoder)
	{
		_cam->beforeRenderEntity(entity, encoder);
		for (auto& comp : _components)
		{
			comp->beforeRenderEntity(entity, encoder);
		}
	}

	Renderable::Renderable(const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Material>& material) noexcept
		: _mesh(mesh)
		, _material(material)
		, _enabled(true)
	{
	}

	Renderable::Renderable(const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Program>& program, const std::shared_ptr<Texture>& texture) noexcept
		: Renderable(mesh, std::make_shared<Material>(program, texture))
	{
	}

	Renderable::Renderable(const std::shared_ptr<Material>& material) noexcept
		: Renderable(nullptr, material)
	{
	}

	std::shared_ptr<IMesh> Renderable::getMesh() const noexcept
	{
		return _mesh;
	}

	Renderable& Renderable::setMesh(const std::shared_ptr<IMesh>& mesh) noexcept
	{
		_mesh = mesh;
		return *this;
	}

	std::shared_ptr<Material> Renderable::getMaterial() const noexcept
	{
		return _material;
	}

	Renderable& Renderable::setMaterial(const std::shared_ptr<Material>& material) noexcept
	{
		_material = material;
		return *this;
	}

	bool Renderable::isEnabled() const noexcept
	{
		return _enabled;
	}

	Renderable& Renderable::setEnabled(bool enabled) noexcept
	{
		_enabled = enabled;
		return *this;
	}

	bool Renderable::valid() const noexcept
	{
		return _mesh != nullptr && _material != nullptr && _material->valid();
	}

	void Renderable::render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const
	{
		if (!_enabled)
		{
			return;
		}
		if (!_mesh->render(encoder))
		{
			return;
		}
		_material->renderSubmit(encoder, viewId);
	}
}