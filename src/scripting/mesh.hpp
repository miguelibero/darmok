#pragma once

#include <bgfx/bgfx.h>
#include <memory>
#include <vector>
#include <optional>
#include <darmok/optional_ref.hpp>
#include "sol.hpp"
#include "math.hpp"
#include "scene_fwd.hpp"

namespace darmok
{
    class Mesh;
    class LuaMaterial;

	class LuaMesh final
	{
	public:
		LuaMesh(const std::shared_ptr<Mesh>& mesh) noexcept;

		std::string to_string() const noexcept;

		std::shared_ptr<Mesh> getReal() const noexcept;
		LuaMaterial getMaterial() const noexcept;
		void setMaterial(const LuaMaterial& material) noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<Mesh> _mesh;
	};

	struct MeshCreator;
	struct MeshData;
	struct MeshCreationConfig;
	struct Cube;
	struct Sphere;
	struct Quad;
	struct Line;
	struct Ray;
	class LuaTexture;

	struct LuaMeshCreator final
	{
		using Config = MeshCreationConfig;

		LuaMeshCreator(const bgfx::VertexLayout& layout) noexcept;
		~LuaMeshCreator();

		Config& getConfig() noexcept;
		void setConfig(const Config& config) noexcept;
		bgfx::VertexLayout& getVertexLayout()  noexcept;

		LuaMesh createMesh(const MeshData& meshData) noexcept;
		LuaMesh createCube1() noexcept;
		LuaMesh createCube2(const Cube& cube) noexcept;
		LuaMesh createSphere1() noexcept;
		LuaMesh createSphere2(const Sphere& sphere) noexcept;
		LuaMesh createSphere3(int lod) noexcept;
		LuaMesh createSphere4(const Sphere& sphere, int lod) noexcept;
		LuaMesh createQuad1() noexcept;
		LuaMesh createQuad2(const Quad& quad) noexcept;
		LuaMesh createLineQuad1() noexcept;
		LuaMesh createLineQuad2(const Quad& quad) noexcept;
		LuaMesh createSprite1(const LuaTexture& texture) noexcept;
		LuaMesh createSprite2(const LuaTexture& texture, const VarUvec2& size) noexcept;
		LuaMesh createRay(const Ray& ray) noexcept;
		LuaMesh createLine(const Line& line) noexcept;
		LuaMesh createLines(const std::vector<Line>& lines) noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<MeshCreator> _creator;
	};

	class MeshComponent;

	class LuaMeshComponent final
	{
	public:
		using native_t = MeshComponent;
		const static LuaNativeComponentType native_type = LuaNativeComponentType::Mesh;

		LuaMeshComponent(MeshComponent& comp) noexcept;

		const MeshComponent& getReal() const;
		MeshComponent& getReal();

		std::vector<LuaMesh> getMeshes() const noexcept;
		std::optional<LuaMesh> getMesh() const noexcept;
		void setMeshes(const std::vector<LuaMesh>& meshes) noexcept;
		void setMesh(const LuaMesh& mesh) noexcept;
		void addMesh(const LuaMesh& mesh) noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		OptionalRef<MeshComponent> _comp;
	};
}