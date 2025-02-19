#include <darmok/render_scene.hpp>
#include <darmok/material.hpp>
#include <darmok/mesh.hpp>
#include <darmok/program.hpp>
#include <darmok/camera.hpp>
#include <darmok/shape.hpp>
#include <darmok/texture.hpp>
#include <darmok/reflect_serialize.hpp>

using namespace entt::literals;

namespace darmok
{
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

	Renderable::Renderable(const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Program>& program, const Color& color) noexcept
		: Renderable(mesh, std::make_shared<Material>(program, color))
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

	bgfx::VertexLayout Renderable::getVertexLayout() const noexcept
	{
		if (!_material)
		{
			return {};
		}
		auto prog = _material->getProgram();
		if (!prog)
		{
			return {};
		}
		return prog->getVertexLayout();
	}

	bool Renderable::valid() const noexcept
	{
		return _mesh != nullptr && _material != nullptr && _material->valid();
	}

	bool Renderable::render(bgfx::Encoder& encoder) const
	{
		if (!_enabled)
		{
			return false;
		}
		return _mesh->render(encoder);
	}

	void Renderable::bindMeta()
	{
		ReflectionSerializeUtils::metaSave<Renderable>();
		ReflectionSerializeUtils::metaLoad<Renderable>();
		SceneReflectionUtils::metaEntityComponent<Renderable>("Renderable")
			.ctor()
			.data<&Renderable::_enabled, entt::as_ref_t>("enabled"_hs)
			.data<&Renderable::_mesh, entt::as_ref_t>("mesh"_hs)
			.data<&Renderable::_material, entt::as_ref_t>("material"_hs)
			;
	}
}