#include <darmok/render.hpp>
#include <darmok/material.hpp>
#include <darmok/mesh.hpp>
#include <darmok/program.hpp>

namespace darmok
{
    
	Renderable::Renderable(const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Material>& material) noexcept
		: _mesh(mesh)
		, _material(material)
	{
	}

	Renderable::Renderable(const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Texture>& texture) noexcept
		: _mesh(mesh)
		, _material(std::make_shared<Material>(texture))
	{
	}

	Renderable::Renderable(const std::shared_ptr<Material>& material) noexcept
		: _material(material)
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

	bool Renderable::valid() const noexcept
	{
		return _mesh != nullptr && _material != nullptr && _material->valid();
	}

	void Renderable::render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const
	{
		_mesh->render(encoder, viewId);
		uint64_t state = _material->beforeRender(encoder, viewId);
		encoder.setState(state);
		auto prog = _material->getProgram();
		encoder.submit(viewId, prog->getHandle());
	}
}