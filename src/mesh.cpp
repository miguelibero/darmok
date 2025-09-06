#include <darmok/mesh.hpp>
#include <darmok/program.hpp>
#include <darmok/shape_serialize.hpp>
#include <darmok/optional_ref.hpp>

#ifdef DARMOK_ASSIMP
#include "detail/assimp.hpp"
#include <darmok/mesh_assimp.hpp>
#include <assimp/scene.h>
#endif


namespace darmok
{
	MeshDefinitionFromSourceLoader::MeshDefinitionFromSourceLoader(IMeshSourceLoader& srcLoader, IProgramDefinitionLoader& progDefLoader, bx::AllocatorI& allocator) noexcept
		: FromDefinitionLoader(srcLoader)
		, _progDefLoader{ progDefLoader }
		, _allocator{ allocator }
	{
	}

	MeshDefinitionFromSourceLoader::Result MeshDefinitionFromSourceLoader::create(const protobuf::ExternalMeshSource& external, const protobuf::VertexLayout& layout)
	{
		DataView data{ external.data() };
		auto defPtr = std::make_shared<Mesh::Definition>();
		if (data.empty())
		{
			return defPtr;
		}
		AssimpLoader loader;
		auto result = loader.loadFromMemory(data);
		if (!result)
		{
			return unexpected{ result.error() };
		}
		auto assimpScene = *result.value();
		OptionalRef<aiMesh> assimpMesh;
		for (int i = 0; i < assimpScene.mNumMeshes; ++i)
		{
			auto m = assimpScene.mMeshes[i];
			auto name = m->mName.C_Str();
			if (name == external.name())
			{
				assimpMesh = m;
				break;
			}
		}
		if (!assimpMesh)
		{
			return unexpected<std::string>{ "mesh not found: " + external.name() };
		}
		AssimpMeshDefinitionConverter converter{ *assimpMesh , *defPtr, layout, _allocator };
		auto convResult = converter();
		if (!convResult)
		{
			return unexpected{ convResult.error() };
		}
		return defPtr;
	}

	MeshDefinitionFromSourceLoader::Result MeshDefinitionFromSourceLoader::create(const std::shared_ptr<Mesh::Source>& src)
	{
		if (!src)
		{
			return unexpected<std::string>{ "Mesh source is null" };
		}

		std::shared_ptr<Program::Definition> progDef;
		if (src->has_standard_program())
		{
			progDef = StandardProgramLoader::loadDefinition(src->standard_program());
		}
		if (!progDef && src->has_program_path())
		{
			auto progResult = _progDefLoader(src->program_path());
			if (!progResult)
			{
				return unexpected{ progResult.error() };
			}
			progDef = progResult.value();
		}
		if (!progDef)
		{
			return unexpected<std::string>{ "No program found for mesh source" };
		}

		auto layout = ConstVertexLayoutWrapper{ progDef->varying().vertex() }.getBgfx();

		std::shared_ptr<Mesh::Definition> defPtr;
		MeshConfig config{ .index32 = src->index32() };
		if (src->has_sphere())
		{
			auto shape = protobuf::convert(src->sphere().shape());
			auto def = MeshData{ shape, src->sphere().lod() }.createDefinition(layout, config);
			defPtr = std::make_shared<Mesh::Definition>(std::move(def));
		}
		else if (src->has_cube())
		{
			auto shape = protobuf::convert(src->cube().shape());
			auto def = MeshData{ shape, src->cube().type() }.createDefinition(layout, config);
			defPtr = std::make_shared<Mesh::Definition>(std::move(def));
		}
		else if (src->has_capsule())
		{
			auto shape = protobuf::convert(src->capsule().shape());
			auto def = MeshData{ shape, src->capsule().lod() }.createDefinition(layout, config);
			defPtr = std::make_shared<Mesh::Definition>(std::move(def));
		}
		else if (src->has_rectangle())
		{
			auto shape = protobuf::convert(src->rectangle().shape());
			auto def = MeshData{ shape, src->rectangle().type() }.createDefinition(layout, config);
			defPtr = std::make_shared<Mesh::Definition>(std::move(def));
		}
#ifdef DARMOK_ASSIMP
		else if (src->has_external())
		{
			return create(src->external(), progDef->varying().vertex());
		}
#endif

		if (defPtr)
		{
			return defPtr;
		}
		
		return unexpected<std::string>{ "Unsupported mesh type" };
	}
}