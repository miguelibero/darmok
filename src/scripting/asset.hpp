#pragma once

#include <memory>
#include <string>
#include <vector>
#include <bgfx/bgfx.h>
#include <darmok/color.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/anim.hpp>
#include <darmok/program.hpp>
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

		uint8_t getShininess() const noexcept;
		LuaMaterial& setShininess(uint8_t v) noexcept;

		float getSpecularStrength() const noexcept;
		LuaMaterial& setSpecularStrength(float v) noexcept;

		static void configure(sol::state_view& lua) noexcept;

	private:
		std::shared_ptr<Material> _material;
	};

	class Mesh;
	struct MeshCreationConfig;
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

		const std::shared_ptr<Mesh>& getReal() const noexcept;
		LuaMaterial getMaterial() const noexcept;
		void setMaterial(const LuaMaterial& material) noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<Mesh> _mesh;
	};

	struct MeshCreator;
	struct MeshCreationConfig;
	struct MeshData;

	struct LuaMeshCreator final
	{
		using Config = MeshCreationConfig;

		LuaMeshCreator(const bgfx::VertexLayout& layout) noexcept;
		LuaMeshCreator(const bgfx::VertexLayout& layout, const Config& cfg) noexcept;
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
		LuaMesh createSprite(const LuaTexture& texture) noexcept;
		LuaMesh createRay(const Ray& ray) noexcept;
		LuaMesh createLine(const Line& line) noexcept;
		LuaMesh createLines(const std::vector<Line>& lines) noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<MeshCreator> _creator;
	};

	class TextureAtlas;
	struct MeshCreationConfig;

	class LuaTextureAtlas final
	{
	public:
		LuaTextureAtlas(const std::shared_ptr<TextureAtlas>& atlas) noexcept;
		const std::shared_ptr<TextureAtlas>& getReal() const noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<TextureAtlas> _atlas;
	};

	struct TextureAtlasMeshCreator;

	struct LuaTextureAtlasMeshCreator final
	{
		using Config = MeshCreationConfig;

		LuaTextureAtlasMeshCreator(const bgfx::VertexLayout& layout, const LuaTextureAtlas& atlas) noexcept;
		LuaTextureAtlasMeshCreator(const bgfx::VertexLayout& layout, const LuaTextureAtlas& atlas, const Config& cfg) noexcept;
		~LuaTextureAtlasMeshCreator();

		Config& getConfig() noexcept;
		void setConfig(const Config& config) noexcept;
		bgfx::VertexLayout& getVertexLayout()  noexcept;
		const LuaTextureAtlas& getTextureAtlas() noexcept;

		LuaMesh createSprite(const std::string& name) const noexcept;
		std::vector<AnimationFrame> createAnimation1(const std::string& namePrefix) const noexcept;
		std::vector<AnimationFrame> createAnimation2(const std::string& namePrefix, float frameDuration) const noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		LuaTextureAtlas _atlas;
		std::shared_ptr<TextureAtlasMeshCreator> _creator;
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
		LuaProgram loadStandardProgram(StandardProgramType type);
		LuaTexture loadTexture(const std::string& name);
		LuaTexture loadColorTexture(const Color& color);
		LuaTextureAtlas loadTextureAtlas(const std::string& name);
		LuaModel loadModel(const std::string& name);

		static void configure(sol::state_view& lua) noexcept;

	private:
		OptionalRef<AssetContext> _assets;
	};
}