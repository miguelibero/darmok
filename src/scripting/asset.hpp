#pragma once

#include <memory>
#include <string>
#include <vector>
#include <bgfx/bgfx.h>
#include <darmok/color.hpp>
#include <darmok/optional_ref.hpp>
#include "sol.hpp"

namespace darmok
{
    class Program;

    class LuaProgram final
	{
	public:
		LuaProgram(const std::shared_ptr<Program>& program) noexcept;
		const bgfx::VertexLayout& getVertexLayout() const noexcept;
		const std::shared_ptr<Program>& getReal() const noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<Program> _program;
	};

    class Texture;

	class LuaTexture final
	{
	public:
		LuaTexture(const std::shared_ptr<Texture>& texture) noexcept;
		const std::shared_ptr<Texture>& getReal() const noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<Texture> _texture;
	};

    class Material;

	class LuaMaterial final
	{
	public:
		LuaMaterial() noexcept;
		LuaMaterial(const std::shared_ptr<Material>& material) noexcept;
		LuaMaterial(const LuaTexture& texture) noexcept;
		const std::shared_ptr<Material>& getReal() const noexcept;

		static void configure(sol::state_view& lua) noexcept;

	private:
		std::shared_ptr<Material> _material;
	};

	class Mesh;
	struct Cube;
	struct Sphere;
	struct Quad;
	struct Line;
	struct Ray;

	class LuaMesh final
	{
	public:
		LuaMesh(const bgfx::VertexLayout& layout) noexcept;
		LuaMesh(const std::shared_ptr<Mesh>& mesh) noexcept;
		static LuaMesh createCube1(const bgfx::VertexLayout& layout) noexcept;
		static LuaMesh createCube2(const bgfx::VertexLayout& layout, const Cube& cube) noexcept;
		static LuaMesh createSphere1(const bgfx::VertexLayout& layout) noexcept;
		static LuaMesh createSphere2(const bgfx::VertexLayout& layout, const Sphere& sphere) noexcept;
		static LuaMesh createSphere3(const bgfx::VertexLayout& layout, const Sphere& sphere, int lod) noexcept;
		static LuaMesh createQuad1(const bgfx::VertexLayout& layout) noexcept;
		static LuaMesh createQuad2(const bgfx::VertexLayout& layout, const Quad& quad) noexcept;
		static LuaMesh createLineQuad1(const bgfx::VertexLayout& layout) noexcept;
		static LuaMesh createLineQuad2(const bgfx::VertexLayout& layout, const Quad& quad) noexcept;
		static LuaMesh createSprite1(const bgfx::VertexLayout& layout, const LuaTexture& texture) noexcept;
		static LuaMesh createSprite2(const bgfx::VertexLayout& layout, const LuaTexture& texture, float scale) noexcept;
		static LuaMesh createLine(const bgfx::VertexLayout& layout, const Line& line) noexcept;
		static LuaMesh createLines(const bgfx::VertexLayout& layout, const std::vector<Line>& lines) noexcept;
		static LuaMesh createRay(const bgfx::VertexLayout& layout, const Ray& ray) noexcept;

		const std::shared_ptr<Mesh>& getReal() const noexcept;
		LuaMaterial getMaterial() const noexcept;
		void setMaterial(const LuaMaterial& material) noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<Mesh> _mesh;
	};

	class Model;
	class LuaEntity;
	class LuaScene;

	class LuaModel final
	{
	public:
		LuaModel(const std::shared_ptr<Model>& model) noexcept;
		const std::shared_ptr<Model>& getReal() const noexcept;

		LuaEntity addToScene1(LuaScene& scene, const bgfx::VertexLayout& layout);
		LuaEntity addToScene2(LuaScene& scene, const bgfx::VertexLayout& layout, sol::protected_function callback);
		LuaEntity addToScene3(LuaScene& scene, const bgfx::VertexLayout& layout, const LuaEntity& parent);
		LuaEntity addToScene4(LuaScene& scene, const bgfx::VertexLayout& layout, const LuaEntity& parent, sol::protected_function callback);

		static void configure(sol::state_view& lua) noexcept;

	private:
		std::shared_ptr<Model> _model;
	};

	class AssetContext;

	class LuaAssets final
	{
	public:
		LuaAssets(AssetContext& assets) noexcept;
		LuaProgram loadProgram(const std::string& name);
		LuaProgram loadStandardProgram(const std::string& name);
		LuaTexture loadColorTexture(const Color& color);
		LuaModel loadModel(const std::string& name);

		static void configure(sol::state_view& lua) noexcept;

	private:
		OptionalRef<AssetContext> _assets;
	};
}